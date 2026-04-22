// Copyright (c) 2025 HIGH CODE LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "meshtastic_phoneapi.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "meshtastic_mesh.h"

static const char *TAG = "MESHTASTIC_PHONEAPI";

#define PA_NONCE_ONLY_CONFIG  69420
#define PA_NONCE_ONLY_NODES   69421
#define PA_MAX_FRAME_SIZE     512
#define PA_FROMRADIO_QUEUE_SZ 16
#define PA_NUM_CONFIGS        10
#define PA_NUM_MODULECONFIGS  16
#define PA_MAX_CHANNELS       8
#define PA_MUTEX_TIMEOUT_MS   50
#define PA_PREBURST_FRAMES    8
#define PA_MIN_APP_VERSION    30200
#define PA_HW_MODEL_PRIVATE   255
#define PA_ROLE_REPEATER      4

typedef struct {
  bool is_used;
  uint16_t len;
  uint8_t buf[PA_MAX_FRAME_SIZE];
} pa_frame_t;

static pa_frame_t s_queue[PA_FROMRADIO_QUEUE_SZ];
static uint8_t s_queue_head = 0;
static uint8_t s_queue_tail = 0;
static SemaphoreHandle_t s_queue_mutex = NULL;

static uint32_t s_node_num = 0;
static phoneapi_state_t s_state = PA_STATE_IDLE;
static uint32_t s_radio_id = 1;
static uint32_t s_want_config_nonce = 0;
static uint8_t s_config_iter = 0;

static uint16_t enc_varint(uint8_t *buf, uint64_t value) {
  uint16_t pos = 0;
  do {
    uint8_t byte = value & 0x7F;
    value >>= 7;
    if (value)
      byte |= 0x80;
    buf[pos++] = byte;
  } while (value);
  return pos;
}

static uint16_t enc_field_varint(uint8_t *buf, uint8_t field_num, uint64_t value) {
  buf[0] = (field_num << 3) | 0;
  return 1 + enc_varint(&buf[1], value);
}

static uint16_t
enc_field_bytes(uint8_t *buf, uint8_t field_num, const uint8_t *data, uint16_t len) {
  buf[0] = (field_num << 3) | 2;
  uint16_t pos = 1 + enc_varint(&buf[1], len);
  memcpy(&buf[pos], data, len);
  return pos + len;
}

static uint64_t dec_varint(const uint8_t *buf, uint16_t max_len, uint16_t *out_used) {
  uint64_t value = 0;
  uint16_t shift = 0;
  uint16_t i = 0;
  while (i < max_len && i < 10) {
    uint8_t byte = buf[i++];
    value |= ((uint64_t)(byte & 0x7F)) << shift;
    if (!(byte & 0x80))
      break;
    shift += 7;
  }
  if (out_used != NULL)
    *out_used = i;
  return value;
}

static bool queue_push(const uint8_t *buf, uint16_t len) {
  if (len == 0 || len > PA_MAX_FRAME_SIZE)
    return false;
  if (xSemaphoreTake(s_queue_mutex, pdMS_TO_TICKS(PA_MUTEX_TIMEOUT_MS)) != pdTRUE) {
    return false;
  }

  uint8_t next = (s_queue_tail + 1) % PA_FROMRADIO_QUEUE_SZ;
  if (next == s_queue_head) {
    s_queue_head = (s_queue_head + 1) % PA_FROMRADIO_QUEUE_SZ;
    ESP_LOGW(TAG, "Queue full, dropping oldest frame");
  }
  s_queue[s_queue_tail].is_used = true;
  s_queue[s_queue_tail].len = len;
  memcpy(s_queue[s_queue_tail].buf, buf, len);
  s_queue_tail = next;

  xSemaphoreGive(s_queue_mutex);
  return true;
}

static uint16_t queue_pop(uint8_t *out_buf, uint16_t max_len) {
  if (xSemaphoreTake(s_queue_mutex, pdMS_TO_TICKS(PA_MUTEX_TIMEOUT_MS)) != pdTRUE) {
    return 0;
  }

  if (s_queue_head == s_queue_tail) {
    xSemaphoreGive(s_queue_mutex);
    return 0;
  }
  uint16_t length = s_queue[s_queue_head].len;
  if (length > max_len)
    length = max_len;
  memcpy(out_buf, s_queue[s_queue_head].buf, length);
  s_queue[s_queue_head].is_used = false;
  s_queue_head = (s_queue_head + 1) % PA_FROMRADIO_QUEUE_SZ;

  xSemaphoreGive(s_queue_mutex);
  return length;
}

static bool queue_has_data(void) {
  bool has = false;
  if (xSemaphoreTake(s_queue_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    has = (s_queue_head != s_queue_tail);
    xSemaphoreGive(s_queue_mutex);
  }
  return has;
}

static uint16_t build_my_info(uint8_t *out, uint32_t rid) {
  uint8_t mi[64];
  uint16_t mi_len = 0;
  mi_len += enc_field_varint(&mi[mi_len], 1, s_node_num);
  mi_len += enc_field_varint(&mi[mi_len], 11, PA_MIN_APP_VERSION);
  const char *pio_env = "highboy-s3";
  mi_len += enc_field_bytes(&mi[mi_len], 13, (const uint8_t *)pio_env, (uint16_t)strlen(pio_env));

  uint16_t pos = 0;
  pos += enc_field_varint(&out[pos], 1, rid);
  pos += enc_field_bytes(&out[pos], 3, mi, mi_len);
  return pos;
}

static uint16_t build_metadata(uint8_t *out, uint32_t rid) {
  uint8_t md[64];
  uint16_t md_len = 0;
  const char *version = "2.7.0-highboy";
  md_len += enc_field_bytes(&md[md_len], 1, (const uint8_t *)version, (uint16_t)strlen(version));
  md_len += enc_field_varint(&md[md_len], 3, 4);
  md_len += enc_field_varint(&md[md_len], 4, 1);
  md_len += enc_field_varint(&md[md_len], 5, 1);
  md_len += enc_field_varint(&md[md_len], 6, 1);
  md_len += enc_field_varint(&md[md_len], 8, PA_HW_MODEL_PRIVATE);
  md_len += enc_field_varint(&md[md_len], 9, PA_ROLE_REPEATER);
  md_len += enc_field_varint(&md[md_len], 10, 0);

  uint16_t pos = 0;
  pos += enc_field_varint(&out[pos], 1, rid);
  pos += enc_field_bytes(&out[pos], 13, md, md_len);
  return pos;
}

static uint16_t build_own_nodeinfo(uint8_t *out, uint32_t rid) {
  uint8_t user[64];
  uint16_t u_len = 0;
  char id_str[16];
  snprintf(id_str, sizeof(id_str), "!%08lx", (unsigned long)s_node_num);
  u_len += enc_field_bytes(&user[u_len], 1, (const uint8_t *)id_str, (uint16_t)strlen(id_str));
  u_len += enc_field_bytes(&user[u_len], 2, (const uint8_t *)"Highboy S3", 10);
  u_len += enc_field_bytes(&user[u_len], 3, (const uint8_t *)"HBS3", 4);
  u_len += enc_field_varint(&user[u_len], 5, PA_HW_MODEL_PRIVATE);

  uint8_t ni[96];
  uint16_t ni_len = 0;
  ni_len += enc_field_varint(&ni[ni_len], 1, s_node_num);
  ni_len += enc_field_bytes(&ni[ni_len], 4, user, u_len);
  ni_len += enc_field_varint(&ni[ni_len], 7, 1);
  ni_len += enc_field_varint(&ni[ni_len], 8, 99);

  uint16_t pos = 0;
  pos += enc_field_varint(&out[pos], 1, rid);
  pos += enc_field_bytes(&out[pos], 4, ni, ni_len);
  return pos;
}

static uint16_t build_config_empty(uint8_t *out, uint32_t rid, uint8_t cfg_type) {
  uint8_t cfg[8];
  uint16_t cfg_len = 0;

  if (cfg_type == 6) {
    uint8_t lora[8];
    uint16_t ll = 0;
    ll += enc_field_varint(&lora[ll], 1, 1);
    ll += enc_field_varint(&lora[ll], 2, 0);
    ll += enc_field_varint(&lora[ll], 7, 1);
    ll += enc_field_varint(&lora[ll], 8, 3);
    ll += enc_field_varint(&lora[ll], 9, 1);
    ll += enc_field_varint(&lora[ll], 10, 22);
    cfg[cfg_len++] = (cfg_type << 3) | 2;
    cfg_len += enc_varint(&cfg[cfg_len], ll);
    memcpy(&cfg[cfg_len], lora, ll);
    cfg_len += ll;
  } else {
    cfg[cfg_len++] = (cfg_type << 3) | 2;
    cfg[cfg_len++] = 0;
  }

  uint16_t pos = 0;
  pos += enc_field_varint(&out[pos], 1, rid);
  pos += enc_field_bytes(&out[pos], 5, cfg, cfg_len);
  return pos;
}

static uint16_t build_moduleconfig_empty(uint8_t *out, uint32_t rid, uint8_t mc_type) {
  uint8_t mc[8];
  uint16_t mc_len = 0;
  mc[mc_len++] = (mc_type << 3) | 2;
  mc[mc_len++] = 0;

  uint16_t pos = 0;
  pos += enc_field_varint(&out[pos], 1, rid);
  pos += enc_field_bytes(&out[pos], 9, mc, mc_len);
  return pos;
}

static uint16_t build_channel(uint8_t *out, uint32_t rid, uint8_t idx) {
  uint8_t ch[32];
  uint16_t cl = 0;
  cl += enc_field_varint(&ch[cl], 1, idx);
  cl += enc_field_varint(&ch[cl], 3, idx == 0 ? 1 : 0);

  uint16_t pos = 0;
  pos += enc_field_varint(&out[pos], 1, rid);
  pos += enc_field_bytes(&out[pos], 10, ch, cl);
  return pos;
}

static uint16_t build_config_complete(uint8_t *out, uint32_t rid, uint32_t nonce) {
  uint16_t pos = 0;
  pos += enc_field_varint(&out[pos], 1, rid);
  pos += enc_field_varint(&out[pos], 7, nonce);
  return pos;
}

static void advance_fsm(void) {
  uint8_t frame[PA_MAX_FRAME_SIZE];
  uint16_t len = 0;

  switch (s_state) {
    case PA_STATE_SEND_MY_INFO:
      len = build_my_info(frame, s_radio_id++);
      s_state = PA_STATE_SEND_METADATA;
      break;

    case PA_STATE_SEND_METADATA:
      len = build_metadata(frame, s_radio_id++);
      s_state = PA_STATE_SEND_CHANNELS;
      s_config_iter = 0;
      break;

    case PA_STATE_SEND_CHANNELS:
      len = build_channel(frame, s_radio_id++, s_config_iter);
      s_config_iter++;
      if (s_config_iter >= PA_MAX_CHANNELS) {
        s_state = PA_STATE_SEND_CONFIG;
        s_config_iter = 1;
      }
      break;

    case PA_STATE_SEND_CONFIG:
      len = build_config_empty(frame, s_radio_id++, s_config_iter);
      s_config_iter++;
      if (s_config_iter > PA_NUM_CONFIGS) {
        s_state = PA_STATE_SEND_MODULECONFIG;
        s_config_iter = 1;
      }
      break;

    case PA_STATE_SEND_MODULECONFIG:
      len = build_moduleconfig_empty(frame, s_radio_id++, s_config_iter);
      s_config_iter++;
      if (s_config_iter > PA_NUM_MODULECONFIGS) {
        s_state = PA_STATE_SEND_OWN_NODEINFO;
      }
      break;

    case PA_STATE_SEND_OWN_NODEINFO:
      len = build_own_nodeinfo(frame, s_radio_id++);
      if (s_want_config_nonce == PA_NONCE_ONLY_CONFIG) {
        s_state = PA_STATE_SEND_COMPLETE_ID;
      } else {
        s_state = PA_STATE_SEND_OTHER_NODEINFOS;
      }
      break;

    case PA_STATE_SEND_OTHER_NODEINFOS:
      s_state = PA_STATE_SEND_COMPLETE_ID;
      advance_fsm();
      return;

    case PA_STATE_SEND_COMPLETE_ID:
      len = build_config_complete(frame, s_radio_id++, s_want_config_nonce);
      s_state = PA_STATE_SEND_PACKETS;
      ESP_LOGI(TAG,
               "Handshake complete (nonce=%lu) — entering PACKETS",
               (unsigned long)s_want_config_nonce);
      break;

    case PA_STATE_SEND_PACKETS:
    case PA_STATE_IDLE:
    default:
      return;
  }

  if (len > 0) {
    queue_push(frame, len);
  }
}

esp_err_t phoneapi_init(uint32_t node_num) {
  s_node_num = node_num;
  memset(s_queue, 0, sizeof(s_queue));
  s_queue_head = 0;
  s_queue_tail = 0;
  s_state = PA_STATE_IDLE;
  s_radio_id = 1;
  s_want_config_nonce = 0;
  s_config_iter = 0;

  if (s_queue_mutex == NULL) {
    s_queue_mutex = xSemaphoreCreateMutex();
    if (s_queue_mutex == NULL)
      return ESP_ERR_NO_MEM;
  }

  ESP_LOGI(
      TAG, "Initialized — node=0x%08lX, queue=%d", (unsigned long)node_num, PA_FROMRADIO_QUEUE_SZ);
  return ESP_OK;
}

esp_err_t phoneapi_on_toradio(const uint8_t *pb_data, uint16_t pb_len) {
  if (pb_data == NULL || pb_len < 2)
    return ESP_ERR_INVALID_ARG;

  uint16_t i = 0;
  while (i < pb_len) {
    uint16_t used = 0;
    uint64_t tag = dec_varint(&pb_data[i], pb_len - i, &used);
    if (used == 0)
      break;
    i += used;
    uint32_t field = (uint32_t)(tag >> 3);
    uint32_t wire_type = (uint32_t)(tag & 0x07);

    if (wire_type == 2) {
      uint16_t len_used = 0;
      uint32_t length = (uint32_t)dec_varint(&pb_data[i], pb_len - i, &len_used);
      i += len_used;
      if (i + length > pb_len)
        break;

      if (field == 1) {
        ESP_LOGI(TAG, "ToRadio.packet %lu bytes -> mesh_send", (unsigned long)length);
        meshtastic_mesh_send(&pb_data[i], (uint16_t)length);
      }
      i += length;
    } else if (wire_type == 0) {
      uint16_t vused = 0;
      uint64_t value = dec_varint(&pb_data[i], pb_len - i, &vused);
      i += vused;
      if (field == 3) {
        s_want_config_nonce = (uint32_t)value;
        ESP_LOGI(TAG, "ToRadio.want_config_id = %lu", (unsigned long)value);
        s_state = PA_STATE_SEND_MY_INFO;
        s_config_iter = 0;
        for (int k = 0;
             k < PA_PREBURST_FRAMES && s_state != PA_STATE_SEND_PACKETS && s_state != PA_STATE_IDLE;
             k++) {
          advance_fsm();
        }
      }
    } else if (wire_type == 5) {
      i += 4;
    } else {
      break;
    }
  }
  return ESP_OK;
}

uint16_t phoneapi_poll_fromradio(uint8_t *out_buf, uint16_t max_len) {
  while (!queue_has_data() && s_state != PA_STATE_SEND_PACKETS && s_state != PA_STATE_IDLE) {
    advance_fsm();
  }
  return queue_pop(out_buf, max_len);
}

bool phoneapi_has_data(void) {
  if (queue_has_data())
    return true;
  if (s_state != PA_STATE_SEND_PACKETS && s_state != PA_STATE_IDLE) {
    return true;
  }
  return false;
}

void phoneapi_push_packet(const uint8_t *mp_bytes, uint16_t mp_len) {
  uint8_t frame[PA_MAX_FRAME_SIZE];
  uint16_t pos = 0;
  pos += enc_field_varint(&frame[pos], 1, s_radio_id++);
  pos += enc_field_bytes(&frame[pos], 2, mp_bytes, mp_len);
  queue_push(frame, pos);
}

void phoneapi_disconnect(void) {
  ESP_LOGI(TAG, "Transport disconnected — resetting FSM");
  s_state = PA_STATE_IDLE;
  s_queue_head = s_queue_tail;
  s_config_iter = 0;
}
