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

#include "mt_mod_nodeinfo.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "meshtastic_mesh.h"
#include "meshtastic_nvs.h"

static const char *TAG = "MT_MOD_NODEINFO";

#define MT_NODEINFO_BROADCAST_SECS   900
#define MT_NODEINFO_REPLY_THROTTLE   43200
#define MT_NODEINFO_HOP_LIMIT        3
#define MT_NODEINFO_HW_MODEL_PRIVATE 255
#define MT_NODEINFO_BCAST_ADDR       0xFFFFFFFF

static uint32_t s_node_num = 0;
static char s_long_name[32] = "Highboy";
static char s_short_name[8] = "HBX";
static uint32_t s_last_broadcast_s = 0;
static uint32_t s_last_reply_s = 0;

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

static uint16_t build_user(uint8_t *out) {
  uint16_t pos = 0;
  char id_str[16];
  snprintf(id_str, sizeof(id_str), "!%08lx", (unsigned long)s_node_num);
  pos += enc_field_bytes(&out[pos], 1, (const uint8_t *)id_str, (uint16_t)strlen(id_str));
  pos += enc_field_bytes(&out[pos], 2, (const uint8_t *)s_long_name, (uint16_t)strlen(s_long_name));
  pos +=
      enc_field_bytes(&out[pos], 3, (const uint8_t *)s_short_name, (uint16_t)strlen(s_short_name));
  pos += enc_field_varint(&out[pos], 5, MT_NODEINFO_HW_MODEL_PRIVATE);
  return pos;
}

void mt_mod_nodeinfo_init(uint32_t node_num) {
  s_node_num = node_num;

  char stored[32] = {0};
  int read_len = mt_nvs_get_blob("long_name", stored, sizeof(stored) - 1);
  if (read_len > 0) {
    memcpy(s_long_name, stored, read_len);
    s_long_name[read_len] = 0;
  }

  memset(stored, 0, sizeof(stored));
  read_len = mt_nvs_get_blob("short_name", stored, sizeof(stored) - 1);
  if (read_len > 0 && read_len < (int)sizeof(s_short_name)) {
    memcpy(s_short_name, stored, read_len);
    s_short_name[read_len] = 0;
  }

  ESP_LOGI(TAG,
           "Initialized — '%s' / '%s' (interval=%us)",
           s_long_name,
           s_short_name,
           MT_NODEINFO_BROADCAST_SECS);
}

void mt_mod_nodeinfo_on_received(const mt_packet_meta_t *meta,
                                 const uint8_t *payload,
                                 uint16_t len) {
  (void)payload;
  ESP_LOGI(TAG, "Received NodeInfo from=0x%08lX (%u bytes)", (unsigned long)meta->from, len);

  if (!meta->want_response)
    return;

  uint32_t now_s = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
  if (s_last_reply_s != 0 && now_s - s_last_reply_s < MT_NODEINFO_REPLY_THROTTLE) {
    ESP_LOGD(TAG, "Reply throttled (last %lus ago)", (unsigned long)(now_s - s_last_reply_s));
    return;
  }
  s_last_reply_s = now_s;

  uint8_t user[96];
  uint16_t ulen = build_user(user);
  ESP_LOGI(TAG,
           "Replying NodeInfo -> 0x%08lX req_id=0x%08lX",
           (unsigned long)meta->from,
           (unsigned long)meta->id);
  meshtastic_mesh_send_data(meta->from,
                            meta->channel,
                            MT_NODEINFO_HOP_LIMIT,
                            MT_PORT_NODEINFO,
                            user,
                            ulen,
                            meta->id,
                            false);
}

void mt_mod_nodeinfo_tick(uint32_t now_s) {
  if (now_s - s_last_broadcast_s < MT_NODEINFO_BROADCAST_SECS)
    return;
  s_last_broadcast_s = now_s;

  uint8_t user[96];
  uint16_t ulen = build_user(user);
  ESP_LOGI(TAG, "Broadcast NodeInfo: '%s' (%s)", s_long_name, s_short_name);
  meshtastic_mesh_send_data(
      MT_NODEINFO_BCAST_ADDR, 0, MT_NODEINFO_HOP_LIMIT, MT_PORT_NODEINFO, user, ulen, 0, false);
}

void mt_mod_nodeinfo_set_name(const char *long_name, const char *short_name) {
  if (long_name != NULL) {
    strncpy(s_long_name, long_name, sizeof(s_long_name) - 1);
    s_long_name[sizeof(s_long_name) - 1] = 0;
    mt_nvs_set_blob("long_name", s_long_name, strlen(s_long_name));
  }
  if (short_name != NULL) {
    strncpy(s_short_name, short_name, sizeof(s_short_name) - 1);
    s_short_name[sizeof(s_short_name) - 1] = 0;
    mt_nvs_set_blob("short_name", s_short_name, strlen(s_short_name));
  }
  s_last_broadcast_s = 0;
  ESP_LOGI(TAG, "Names updated — '%s' / '%s'", s_long_name, s_short_name);
}

const char *mt_mod_nodeinfo_get_long(void) {
  return s_long_name;
}
const char *mt_mod_nodeinfo_get_short(void) {
  return s_short_name;
}
