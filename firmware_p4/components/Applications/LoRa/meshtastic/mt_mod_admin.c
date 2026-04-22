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

#include "mt_mod_admin.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "meshtastic_mesh.h"
#include "meshtastic_nvs.h"
#include "mt_mod_nodeinfo.h"
#include "mt_mod_position.h"

static const char *TAG = "MT_MOD_ADMIN";

#define MT_ADMIN_PASSKEY_TTL_SECS  300
#define MT_ADMIN_PASSKEY_SIZE      8
#define MT_ADMIN_HOP_LIMIT         3
#define MT_ADMIN_HW_MODEL_PRIVATE  255
#define MT_ADMIN_ROLE_REPEATER     4
#define MT_ADMIN_FIRST_SET_VARIANT 32
#define MT_ADMIN_REBOOT_DELAY_MS   1000

#define MT_ADMIN_VARIANT_GET_OWNER           1
#define MT_ADMIN_VARIANT_GET_CONFIG          3
#define MT_ADMIN_VARIANT_GET_CONFIG_RESPONSE 4
#define MT_ADMIN_VARIANT_GET_DEVICE_METADATA 8
#define MT_ADMIN_VARIANT_SET_OWNER           32
#define MT_ADMIN_VARIANT_SET_CHANNEL         35
#define MT_ADMIN_VARIANT_SET_CONFIG          36
#define MT_ADMIN_VARIANT_REBOOT              50
#define MT_ADMIN_VARIANT_SHUTDOWN            51
#define MT_ADMIN_VARIANT_FACTORY_RESET_CFG   52
#define MT_ADMIN_VARIANT_FACTORY_RESET_DEV   53
#define MT_ADMIN_VARIANT_SET_FIXED_POSITION  54
#define MT_ADMIN_VARIANT_REMOVE_FIXED_POS    55
#define MT_ADMIN_VARIANT_SET_FAVORITE        56
#define MT_ADMIN_VARIANT_REMOVE_FAVORITE     57
#define MT_ADMIN_VARIANT_BEGIN_EDIT          59
#define MT_ADMIN_VARIANT_COMMIT_EDIT         60
#define MT_ADMIN_VARIANT_SESSION_PASSKEY     101

static uint8_t s_session_passkey[MT_ADMIN_PASSKEY_SIZE];
static uint32_t s_passkey_set_s = 0;
static uint32_t s_node_num = 0;
static bool s_is_edit_transaction_open = false;
static uint32_t s_reboot_at_ms = 0;

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

static void regen_passkey(uint32_t now_s) {
  for (int i = 0; i < MT_ADMIN_PASSKEY_SIZE; i++) {
    s_session_passkey[i] = (uint8_t)(esp_random() & 0xFF);
  }
  s_passkey_set_s = now_s;
}

static bool is_passkey_valid(const uint8_t *provided, uint16_t provided_len) {
  uint32_t now_s = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
  if (now_s - s_passkey_set_s > MT_ADMIN_PASSKEY_TTL_SECS)
    return false;
  if (provided_len != MT_ADMIN_PASSKEY_SIZE)
    return false;
  return memcmp(provided, s_session_passkey, MT_ADMIN_PASSKEY_SIZE) == 0;
}

static uint16_t build_user(uint8_t *out) {
  uint16_t pos = 0;
  char id_str[16];
  snprintf(id_str, sizeof(id_str), "!%08lx", (unsigned long)s_node_num);
  pos += enc_field_bytes(&out[pos], 1, (const uint8_t *)id_str, (uint16_t)strlen(id_str));
  const char *long_name = mt_mod_nodeinfo_get_long();
  const char *short_name = mt_mod_nodeinfo_get_short();
  pos += enc_field_bytes(&out[pos], 2, (const uint8_t *)long_name, (uint16_t)strlen(long_name));
  pos += enc_field_bytes(&out[pos], 3, (const uint8_t *)short_name, (uint16_t)strlen(short_name));
  pos += enc_field_varint(&out[pos], 5, MT_ADMIN_HW_MODEL_PRIVATE);
  return pos;
}

static uint16_t wrap_admin(uint8_t *out,
                           uint8_t variant_field,
                           const uint8_t *variant_data,
                           uint16_t variant_len,
                           bool include_passkey,
                           uint32_t now_s) {
  uint16_t pos = 0;
  pos += enc_field_bytes(&out[pos], variant_field, variant_data, variant_len);
  if (include_passkey) {
    regen_passkey(now_s);
    pos += enc_field_bytes(
        &out[pos], MT_ADMIN_VARIANT_SESSION_PASSKEY, s_session_passkey, MT_ADMIN_PASSKEY_SIZE);
  }
  return pos;
}

static void handle_get_owner(const mt_packet_meta_t *meta, uint32_t now_s) {
  uint8_t user[96];
  uint16_t ulen = build_user(user);

  uint8_t admin[128];
  uint16_t alen = wrap_admin(admin, MT_ADMIN_VARIANT_GET_OWNER, user, ulen, true, now_s);

  ESP_LOGI(TAG, "get_owner -> 0x%08lX", (unsigned long)meta->from);
  meshtastic_mesh_send_data(
      meta->from, meta->channel, MT_ADMIN_HOP_LIMIT, MT_PORT_ADMIN, admin, alen, meta->id, false);
}

static void
handle_set_owner(const mt_packet_meta_t *meta, const uint8_t *user_bytes, uint16_t user_len) {
  (void)meta;
  char long_name[32] = {0};
  char short_name[8] = {0};

  uint16_t i = 0;
  while (i < user_len) {
    uint16_t used = 0;
    uint64_t tag = dec_varint(&user_bytes[i], user_len - i, &used);
    if (used == 0)
      break;
    i += used;
    uint32_t field = (uint32_t)(tag >> 3);
    uint32_t wire_type = (uint32_t)(tag & 0x07);

    if (wire_type == 2) {
      uint16_t len_used = 0;
      uint32_t length = (uint32_t)dec_varint(&user_bytes[i], user_len - i, &len_used);
      i += len_used;
      if (i + length > user_len)
        break;
      if (field == 2) {
        uint32_t copy = length < sizeof(long_name) - 1 ? length : sizeof(long_name) - 1;
        memcpy(long_name, &user_bytes[i], copy);
        long_name[copy] = 0;
      } else if (field == 3) {
        uint32_t copy = length < sizeof(short_name) - 1 ? length : sizeof(short_name) - 1;
        memcpy(short_name, &user_bytes[i], copy);
        short_name[copy] = 0;
      }
      i += length;
    } else if (wire_type == 0) {
      uint16_t vused = 0;
      dec_varint(&user_bytes[i], user_len - i, &vused);
      i += vused;
    } else {
      break;
    }
  }

  mt_mod_nodeinfo_set_name(long_name[0] ? long_name : NULL, short_name[0] ? short_name : NULL);
  ESP_LOGI(TAG, "set_owner — '%s' / '%s'", long_name, short_name);
}

static void handle_get_device_metadata(const mt_packet_meta_t *meta, uint32_t now_s) {
  uint8_t metadata[96];
  uint16_t mlen = 0;
  const char *version = "2.7.0-highboy";
  mlen += enc_field_bytes(&metadata[mlen], 1, (const uint8_t *)version, (uint16_t)strlen(version));
  mlen += enc_field_varint(&metadata[mlen], 3, 4);
  mlen += enc_field_varint(&metadata[mlen], 4, 1);
  mlen += enc_field_varint(&metadata[mlen], 8, MT_ADMIN_HW_MODEL_PRIVATE);
  mlen += enc_field_varint(&metadata[mlen], 9, MT_ADMIN_ROLE_REPEATER);

  uint8_t admin[128];
  uint16_t alen =
      wrap_admin(admin, MT_ADMIN_VARIANT_GET_DEVICE_METADATA, metadata, mlen, true, now_s);

  ESP_LOGI(TAG, "get_device_metadata -> 0x%08lX", (unsigned long)meta->from);
  meshtastic_mesh_send_data(
      meta->from, meta->channel, MT_ADMIN_HOP_LIMIT, MT_PORT_ADMIN, admin, alen, meta->id, false);
}

static void schedule_reboot(int32_t seconds) {
  ESP_LOGW(TAG, "reboot scheduled in %lds", (long)seconds);
  if (seconds <= 0) {
    esp_restart();
  } else {
    s_reboot_at_ms = xTaskGetTickCount() * portTICK_PERIOD_MS + (seconds * 1000);
  }
}

static void perform_factory_reset(bool erase_bonds) {
  (void)erase_bonds;
  ESP_LOGW(TAG, "factory_reset — erasing NVS");
  mt_nvs_factory_reset();
  s_reboot_at_ms = xTaskGetTickCount() * portTICK_PERIOD_MS + MT_ADMIN_REBOOT_DELAY_MS;
}

static void handle_set_fixed_position(const uint8_t *pos_bytes, uint16_t len) {
  int32_t lat = 0;
  int32_t lon = 0;
  int32_t alt = 0;

  uint16_t i = 0;
  while (i < len) {
    uint16_t used = 0;
    uint64_t tag = dec_varint(&pos_bytes[i], len - i, &used);
    if (used == 0)
      break;
    i += used;
    uint32_t field = (uint32_t)(tag >> 3);
    uint32_t wire_type = (uint32_t)(tag & 0x07);

    if (wire_type == 5) {
      if (i + 4 > len)
        break;
      uint32_t value;
      memcpy(&value, &pos_bytes[i], 4);
      if (field == 1)
        lat = (int32_t)value;
      else if (field == 2)
        lon = (int32_t)value;
      i += 4;
    } else if (wire_type == 0) {
      uint16_t vused = 0;
      uint64_t value = dec_varint(&pos_bytes[i], len - i, &vused);
      i += vused;
      if (field == 3)
        alt = (int32_t)value;
    } else {
      break;
    }
  }
  mt_mod_position_set_fixed(lat, lon, alt);
}

static void dispatch_variant(const mt_packet_meta_t *meta,
                             uint32_t variant_field,
                             uint32_t wire_type,
                             const uint8_t *variant_data,
                             uint16_t variant_len) {
  uint32_t now_s = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
  uint64_t scalar = 0;
  if (wire_type == 0 && variant_data != NULL) {
    uint16_t used = 0;
    scalar = dec_varint(variant_data, variant_len, &used);
  }

  switch (variant_field) {
    case MT_ADMIN_VARIANT_GET_OWNER:
      handle_get_owner(meta, now_s);
      break;
    case MT_ADMIN_VARIANT_GET_CONFIG:
      ESP_LOGI(TAG, "get_config_request type=%lu (partial)", (unsigned long)scalar);
      {
        uint8_t admin[32];
        uint8_t empty = 0;
        uint16_t alen =
            wrap_admin(admin, MT_ADMIN_VARIANT_GET_CONFIG_RESPONSE, &empty, 1, true, now_s);
        meshtastic_mesh_send_data(meta->from,
                                  meta->channel,
                                  MT_ADMIN_HOP_LIMIT,
                                  MT_PORT_ADMIN,
                                  admin,
                                  alen,
                                  meta->id,
                                  false);
      }
      break;
    case MT_ADMIN_VARIANT_GET_DEVICE_METADATA:
      handle_get_device_metadata(meta, now_s);
      break;

    case MT_ADMIN_VARIANT_SET_OWNER:
      handle_set_owner(meta, variant_data, variant_len);
      break;
    case MT_ADMIN_VARIANT_SET_CHANNEL:
      ESP_LOGW(TAG, "set_channel not implemented yet");
      break;
    case MT_ADMIN_VARIANT_SET_CONFIG:
      ESP_LOGW(TAG, "set_config partial");
      break;

    case MT_ADMIN_VARIANT_REBOOT:
      schedule_reboot((int32_t)scalar);
      break;
    case MT_ADMIN_VARIANT_SHUTDOWN:
      ESP_LOGW(TAG, "shutdown_seconds=%lld ignored (no shutdown HW)", (long long)scalar);
      break;
    case MT_ADMIN_VARIANT_FACTORY_RESET_CFG:
      perform_factory_reset(false);
      break;
    case MT_ADMIN_VARIANT_FACTORY_RESET_DEV:
      perform_factory_reset(true);
      break;

    case MT_ADMIN_VARIANT_SET_FIXED_POSITION:
      handle_set_fixed_position(variant_data, variant_len);
      break;
    case MT_ADMIN_VARIANT_REMOVE_FIXED_POS:
      mt_mod_position_remove_fixed();
      break;

    case MT_ADMIN_VARIANT_SET_FAVORITE:
    case MT_ADMIN_VARIANT_REMOVE_FAVORITE:
      ESP_LOGI(TAG,
               "favorite variant=%lu scalar=%llu (TODO)",
               (unsigned long)variant_field,
               (unsigned long long)scalar);
      break;

    case MT_ADMIN_VARIANT_BEGIN_EDIT:
      s_is_edit_transaction_open = true;
      ESP_LOGI(TAG, "begin_edit_settings");
      break;
    case MT_ADMIN_VARIANT_COMMIT_EDIT:
      s_is_edit_transaction_open = false;
      mt_nvs_commit();
      ESP_LOGI(TAG, "commit_edit_settings");
      break;

    default:
      ESP_LOGI(TAG, "Variant %lu not implemented", (unsigned long)variant_field);
      break;
  }
}

void mt_mod_admin_init(uint32_t node_num) {
  s_node_num = node_num;
  memset(s_session_passkey, 0, sizeof(s_session_passkey));
  s_passkey_set_s = 0;
  s_is_edit_transaction_open = false;
  s_reboot_at_ms = 0;
  ESP_LOGI(TAG, "Initialized");
}

void mt_mod_admin_on_received(const mt_packet_meta_t *meta, const uint8_t *payload, uint16_t len) {
  uint32_t variant_field = 0;
  uint32_t wire_type = 0;
  const uint8_t *variant_data = NULL;
  uint16_t variant_len = 0;
  const uint8_t *passkey = NULL;
  uint16_t passkey_len = 0;
  uint8_t scalar_buf[16];

  uint16_t i = 0;
  while (i < len) {
    uint16_t used = 0;
    uint64_t tag = dec_varint(&payload[i], len - i, &used);
    if (used == 0)
      break;
    i += used;
    uint32_t field = (uint32_t)(tag >> 3);
    uint32_t wt = (uint32_t)(tag & 0x07);

    if (wt == 2) {
      uint16_t len_used = 0;
      uint32_t length = (uint32_t)dec_varint(&payload[i], len - i, &len_used);
      i += len_used;
      if (i + length > len)
        break;
      if (field == MT_ADMIN_VARIANT_SESSION_PASSKEY) {
        passkey = &payload[i];
        passkey_len = (uint16_t)length;
      } else {
        variant_field = field;
        wire_type = 2;
        variant_data = &payload[i];
        variant_len = (uint16_t)length;
      }
      i += length;
    } else if (wt == 0) {
      uint16_t vused = 0;
      uint64_t scalar_val = dec_varint(&payload[i], len - i, &vused);
      i += vused;
      variant_field = field;
      wire_type = 0;
      uint16_t scalar_len = enc_varint(scalar_buf, scalar_val);
      variant_data = scalar_buf;
      variant_len = scalar_len;
    } else {
      break;
    }
  }

  bool requires_passkey = (variant_field >= MT_ADMIN_FIRST_SET_VARIANT);
  bool is_local = (meta->from == 0 || meta->from == s_node_num);

  if (requires_passkey && !is_local) {
    if (!is_passkey_valid(passkey, passkey_len)) {
      ESP_LOGW(
          TAG, "Variant=%lu REJECTED (invalid or expired passkey)", (unsigned long)variant_field);
      return;
    }
  }

  dispatch_variant(meta, variant_field, wire_type, variant_data, variant_len);
}

void mt_mod_admin_tick(void) {
  uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
  if (s_reboot_at_ms != 0 && now_ms >= s_reboot_at_ms) {
    ESP_LOGW(TAG, "Scheduled reboot fired");
    esp_restart();
  }
}
