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

#include "meshcore_ble.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "meshcore_nvs.h"

extern void ble_store_config_init(void);

static const char *TAG = "MC_BLE";

#define CMD_APP_START            0x01
#define CMD_SEND_TXT_MSG         0x02
#define CMD_SEND_CHANNEL_TXT_MSG 0x03
#define CMD_GET_CONTACTS         0x04
#define CMD_GET_DEVICE_TIME      0x05
#define CMD_SET_DEVICE_TIME      0x06
#define CMD_SEND_SELF_ADVERT     0x07
#define CMD_SET_ADVERT_NAME      0x08
#define CMD_ADD_UPDATE_CONTACT   0x09
#define CMD_SYNC_NEXT_MESSAGE    0x0A
#define CMD_SET_RADIO_PARAMS     0x0B
#define CMD_SET_RADIO_TX_POWER   0x0C
#define CMD_RESET_PATH           0x0D
#define CMD_SET_ADVERT_LATLON    0x0E
#define CMD_REMOVE_CONTACT       0x0F
#define CMD_SHARE_CONTACT        0x10
#define CMD_EXPORT_CONTACT       0x11
#define CMD_IMPORT_CONTACT       0x12
#define CMD_REBOOT               0x13
#define CMD_GET_BATT_AND_STORAGE 0x14
#define CMD_SET_TUNING_PARAMS    0x15
#define CMD_DEVICE_QUERY         0x16
#define CMD_GET_CONTACT_BY_KEY   0x1E
#define CMD_GET_CHANNEL          0x1F
#define CMD_SET_CHANNEL          0x20
#define CMD_GET_ADVERT_PATH      0x2A
#define CMD_SET_FLOOD_SCOPE      0x36

#define RESP_CODE_OK                  0x00
#define RESP_CODE_ERR                 0x01
#define RESP_CODE_CONTACTS_START      0x02
#define RESP_CODE_CONTACT             0x03
#define RESP_CODE_END_OF_CONTACTS     0x04
#define RESP_CODE_SELF_INFO           0x05
#define RESP_CODE_SENT                0x06
#define RESP_CODE_CONTACT_MSG_RECV    0x07
#define RESP_CODE_CHANNEL_MSG_RECV    0x08
#define RESP_CODE_CURR_TIME           0x09
#define RESP_CODE_NO_MORE_MESSAGES    0x0A
#define RESP_CODE_EXPORT_CONTACT      0x0B
#define RESP_CODE_BATT_AND_STORAGE    0x0C
#define RESP_CODE_DEVICE_INFO         0x0D
#define RESP_CODE_CONTACT_MSG_RECV_V3 0x10
#define RESP_CODE_CHANNEL_MSG_RECV_V3 0x11
#define RESP_CODE_CHANNEL_INFO        0x12
#define RESP_CODE_ADVERT_PATH         0x16

#define PUSH_CODE_ADVERT         0x80
#define PUSH_CODE_PATH_UPDATED   0x81
#define PUSH_CODE_SEND_CONFIRMED 0x82
#define PUSH_CODE_MSG_WAITING    0x83
#define PUSH_CODE_NEW_ADVERT     0x8A

#define ERR_CODE_UNSUPPORTED_CMD 1
#define ERR_CODE_NOT_FOUND       2
#define ERR_CODE_TABLE_FULL      3
#define ERR_CODE_BAD_STATE       4
#define ERR_CODE_FILE_IO_ERROR   5
#define ERR_CODE_ILLEGAL_ARG     6

#define FIRMWARE_VER_CODE 10
#define FIRMWARE_VERSION  "v1.14.1"
#define DEVICE_MODEL      "Highboy-MC-S3"

#define TXT_TYPE_PLAIN 0x00

#define CONTACT_FRAME_SIZE 143

#define MC_OFFLINE_QUEUE_SIZE 16
#define MC_OFFLINE_FRAME_MAX  240

#define MC_NVS_KEY_BLE_PIN "ble_pin"

#define MC_BLE_PIN_MIN 100000U
#define MC_BLE_PIN_MAX 999999U

static const ble_uuid128_t NUS_SERVICE_UUID = BLE_UUID128_INIT(
    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E);
static const ble_uuid128_t NUS_RX_UUID = BLE_UUID128_INIT(
    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E);
static const ble_uuid128_t NUS_TX_UUID = BLE_UUID128_INIT(
    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E);

typedef struct {
  uint16_t len;
  uint8_t buf[MC_OFFLINE_FRAME_MAX];
} mc_queued_frame_t;

static meshcore_ble_config_t s_cfg;
static char s_device_name[32];
static uint8_t s_own_addr_type;
static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t s_tx_char_attr_handle = 0;
static bool s_is_ble_ready = false;
static uint8_t s_app_target_ver = 0;
static uint32_t s_ble_pin = 0;

static mc_queued_frame_t s_offline_queue[MC_OFFLINE_QUEUE_SIZE];
static uint8_t s_offline_queue_len = 0;

static void send_resp(const uint8_t *buf, uint16_t len);
static void send_err(uint8_t sub);
static void send_ok(void);
static void ble_advertise(void);
static int gap_event_cb(struct ble_gap_event *event, void *arg);

static uint32_t load_or_generate_ble_pin(void);

static void offline_queue_push(const uint8_t *frame, uint16_t len);
static bool offline_queue_pop(uint8_t *out, uint16_t *out_len);

static void serialize_contact(uint8_t out[CONTACT_FRAME_SIZE], const meshcore_contact_t *c);
static void deserialize_contact(meshcore_contact_t *c, const uint8_t in[], size_t in_len);
static void write_contact_resp_frame(uint8_t code, const meshcore_contact_t *c);

static void handle_device_query(const uint8_t *p, uint16_t len);
static void handle_app_start(const uint8_t *p, uint16_t len);
static void handle_batt_storage(const uint8_t *p, uint16_t len);
static void handle_sync_next_msg(const uint8_t *p, uint16_t len);
static void handle_get_device_time(const uint8_t *p, uint16_t len);
static void handle_set_device_time(const uint8_t *p, uint16_t len);
static void handle_send_self_advert(const uint8_t *p, uint16_t len);
static void handle_set_advert_name(const uint8_t *p, uint16_t len);
static void handle_set_advert_latlon(const uint8_t *p, uint16_t len);
static void handle_add_update_contact(const uint8_t *p, uint16_t len);
static void handle_get_contact_by_key(const uint8_t *p, uint16_t len);
static void handle_get_contacts(const uint8_t *p, uint16_t len);
static void handle_remove_contact(const uint8_t *p, uint16_t len);
static void handle_share_contact(const uint8_t *p, uint16_t len);
static void handle_export_contact(const uint8_t *p, uint16_t len);
static void handle_import_contact(const uint8_t *p, uint16_t len);
static void handle_reset_path(const uint8_t *p, uint16_t len);
static void handle_get_channel(const uint8_t *p, uint16_t len);
static void handle_set_channel(const uint8_t *p, uint16_t len);
static void handle_get_advert_path(const uint8_t *p, uint16_t len);
static void handle_send_channel_txt(const uint8_t *p, uint16_t len);
static void handle_send_txt_msg(const uint8_t *p, uint16_t len);
static void handle_set_radio_params(const uint8_t *p, uint16_t len);
static void handle_set_radio_tx_power(const uint8_t *p, uint16_t len);

static int rx_char_access(uint16_t conn_handle,
                          uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt,
                          void *arg);
static int tx_char_access(uint16_t conn_handle,
                          uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt,
                          void *arg);

static void on_sync(void);
static void on_reset(int reason);
static void host_task(void *param);

static const struct ble_gatt_chr_def NUS_CHRS[] = {
    {
        .uuid = &NUS_RX_UUID.u,
        .access_cb = rx_char_access,
        .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_WRITE_ENC |
                 BLE_GATT_CHR_F_WRITE_AUTHEN,
    },
    {
        .uuid = &NUS_TX_UUID.u,
        .access_cb = tx_char_access,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ_ENC |
                 BLE_GATT_CHR_F_READ_AUTHEN,
        .val_handle = &s_tx_char_attr_handle,
    },
    {0}};

static const struct ble_gatt_svc_def NUS_SVCS[] = {{
                                                       .type = BLE_GATT_SVC_TYPE_PRIMARY,
                                                       .uuid = &NUS_SERVICE_UUID.u,
                                                       .characteristics = NUS_CHRS,
                                                   },
                                                   {0}};

esp_err_t meshcore_ble_init(const meshcore_ble_config_t *cfg) {
  if (cfg == NULL)
    return ESP_ERR_INVALID_ARG;
  s_cfg = *cfg;

  uint8_t mac[6] = {0};
  esp_efuse_mac_get_default(mac);
  const char *prefix = cfg->device_name ? cfg->device_name : "Highboy-MC";
  snprintf(s_device_name, sizeof(s_device_name), "%s-%02X%02X", prefix, mac[4], mac[5]);

  s_ble_pin = load_or_generate_ble_pin();

  esp_err_t ret = nimble_port_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "nimble_port_init failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_DISP_ONLY;
  ble_hs_cfg.sm_bonding = 1;
  ble_hs_cfg.sm_mitm = 1;
  ble_hs_cfg.sm_sc = 1;
  ble_hs_cfg.sm_our_key_dist = 3;
  ble_hs_cfg.sm_their_key_dist = 3;

  ble_hs_cfg.reset_cb = on_reset;
  ble_hs_cfg.sync_cb = on_sync;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  ble_store_config_init();

  int rc = ble_gatts_count_cfg(NUS_SVCS);
  if (rc != 0) {
    ESP_LOGE(TAG, "gatts_count_cfg: %d", rc);
    return ESP_FAIL;
  }
  rc = ble_gatts_add_svcs(NUS_SVCS);
  if (rc != 0) {
    ESP_LOGE(TAG, "gatts_add_svcs: %d", rc);
    return ESP_FAIL;
  }

  ble_att_set_preferred_mtu(512);

  ble_svc_gap_device_name_set(s_device_name);

  nimble_port_freertos_init(host_task);

  ESP_LOGI(TAG, "BLE init complete (device='%s' PIN=%lu)", s_device_name, (unsigned long)s_ble_pin);
  return ESP_OK;
}

bool meshcore_ble_is_connected(void) {
  return s_conn_handle != BLE_HS_CONN_HANDLE_NONE;
}

void meshcore_ble_send(const uint8_t *data, uint16_t len) {
  send_resp(data, len);
}

void meshcore_ble_push_new_advert(const meshcore_contact_t *c) {
  uint8_t buf[1 + CONTACT_FRAME_SIZE + 4];
  buf[0] = PUSH_CODE_NEW_ADVERT;
  serialize_contact(&buf[1], c);
  memcpy(&buf[1 + CONTACT_FRAME_SIZE], &c->lastmod, 4);
  send_resp(buf, sizeof(buf));
  ESP_LOGI(TAG, "push NEW_ADVERT name='%s' type=%u", c->name, c->type);
}

void meshcore_ble_push_channel_msg(
    uint8_t channel_idx, uint8_t path_len, uint32_t timestamp, int8_t snr, const char *text) {
  uint8_t buf[240];
  uint16_t o = 0;

  if (s_app_target_ver >= 3) {
    buf[o++] = RESP_CODE_CHANNEL_MSG_RECV_V3;
    buf[o++] = (uint8_t)((int8_t)(snr * 4));
    buf[o++] = 0;
    buf[o++] = 0;
  } else {
    buf[o++] = RESP_CODE_CHANNEL_MSG_RECV;
  }

  buf[o++] = channel_idx;
  buf[o++] = path_len;
  buf[o++] = TXT_TYPE_PLAIN;
  memcpy(&buf[o], &timestamp, 4);
  o += 4;

  uint16_t tl = (uint16_t)strlen(text);
  if (o + tl >= sizeof(buf))
    tl = sizeof(buf) - o;
  memcpy(&buf[o], text, tl);
  o += tl;

  offline_queue_push(buf, o);
  uint8_t tickle = PUSH_CODE_MSG_WAITING;
  send_resp(&tickle, 1);
  ESP_LOGI(TAG,
           "queue CHANNEL_MSG %s ch=%u snr=%d text='%s'",
           s_app_target_ver >= 3 ? "V3" : "V1",
           channel_idx,
           snr,
           text);
}

void meshcore_ble_push_contact_msg(const uint8_t peer_pub_key[32],
                                   uint8_t path_len,
                                   uint8_t txt_type,
                                   uint32_t timestamp,
                                   int8_t snr,
                                   const char *text) {
  uint8_t buf[240];
  uint16_t o = 0;

  if (s_app_target_ver >= 3) {
    buf[o++] = RESP_CODE_CONTACT_MSG_RECV_V3;
    buf[o++] = (uint8_t)((int8_t)(snr * 4));
    buf[o++] = 0;
    buf[o++] = 0;
  } else {
    buf[o++] = RESP_CODE_CONTACT_MSG_RECV;
  }

  memcpy(&buf[o], peer_pub_key, 6);
  o += 6;
  buf[o++] = path_len;
  buf[o++] = txt_type;
  memcpy(&buf[o], &timestamp, 4);
  o += 4;

  uint16_t tl = (uint16_t)strlen(text);
  if (o + tl >= sizeof(buf))
    tl = sizeof(buf) - o;
  memcpy(&buf[o], text, tl);
  o += tl;

  offline_queue_push(buf, o);
  uint8_t tickle = PUSH_CODE_MSG_WAITING;
  send_resp(&tickle, 1);
  ESP_LOGI(TAG,
           "queue CONTACT_MSG %s peer=%02X%02X%02X%02X txt_type=%u text='%s'",
           s_app_target_ver >= 3 ? "V3" : "V1",
           peer_pub_key[0],
           peer_pub_key[1],
           peer_pub_key[2],
           peer_pub_key[3],
           txt_type,
           text);
}

void meshcore_ble_push_send_confirmed(uint32_t ack_crc, uint8_t snr) {
  uint8_t buf[6];
  buf[0] = PUSH_CODE_SEND_CONFIRMED;
  buf[1] = (uint8_t)((int8_t)(snr * 4));
  memcpy(&buf[2], &ack_crc, 4);
  send_resp(buf, sizeof(buf));
  ESP_LOGI(TAG, "push SEND_CONFIRMED CRC=0x%08lX", (unsigned long)ack_crc);
}

void meshcore_ble_push_path_updated(const meshcore_contact_t *c) {
  uint8_t buf[1 + CONTACT_FRAME_SIZE + 4];
  buf[0] = PUSH_CODE_PATH_UPDATED;
  serialize_contact(&buf[1], c);
  memcpy(&buf[1 + CONTACT_FRAME_SIZE], &c->lastmod, 4);
  send_resp(buf, sizeof(buf));
}

static void send_resp(const uint8_t *buf, uint16_t len) {
  if (s_conn_handle == BLE_HS_CONN_HANDLE_NONE || s_tx_char_attr_handle == 0) {
    ESP_LOGW(TAG, "No conn/tx handle, drop %u bytes", len);
    return;
  }
  struct os_mbuf *om = ble_hs_mbuf_from_flat(buf, len);
  if (om == NULL) {
    ESP_LOGE(TAG, "mbuf alloc failed");
    return;
  }
  int rc = ble_gatts_notify_custom(s_conn_handle, s_tx_char_attr_handle, om);
  if (rc != 0)
    ESP_LOGW(TAG, "notify failed: %d", rc);
}

static void send_err(uint8_t sub) {
  uint8_t resp[2] = {RESP_CODE_ERR, sub};
  send_resp(resp, 2);
}

static void send_ok(void) {
  uint8_t resp[1] = {RESP_CODE_OK};
  send_resp(resp, 1);
}

static uint32_t load_or_generate_ble_pin(void) {
  uint32_t pin = 0;
  if (mc_nvs_get_u32(MC_NVS_KEY_BLE_PIN, &pin) == ESP_OK && pin >= MC_BLE_PIN_MIN &&
      pin <= MC_BLE_PIN_MAX) {
    return pin;
  }
  pin = MC_BLE_PIN_MIN + (esp_random() % (MC_BLE_PIN_MAX - MC_BLE_PIN_MIN + 1));
  mc_nvs_set_u32(MC_NVS_KEY_BLE_PIN, pin);
  ESP_LOGI(TAG, "BLE PIN generated: %lu", (unsigned long)pin);
  return pin;
}

static void offline_queue_push(const uint8_t *frame, uint16_t len) {
  if (len == 0 || len > MC_OFFLINE_FRAME_MAX)
    return;
  if (s_offline_queue_len >= MC_OFFLINE_QUEUE_SIZE) {
    for (int i = 0; i < MC_OFFLINE_QUEUE_SIZE - 1; i++) {
      s_offline_queue[i] = s_offline_queue[i + 1];
    }
    s_offline_queue_len = MC_OFFLINE_QUEUE_SIZE - 1;
  }
  s_offline_queue[s_offline_queue_len].len = len;
  memcpy(s_offline_queue[s_offline_queue_len].buf, frame, len);
  s_offline_queue_len++;
}

static bool offline_queue_pop(uint8_t *out, uint16_t *out_len) {
  if (s_offline_queue_len == 0)
    return false;
  *out_len = s_offline_queue[0].len;
  memcpy(out, s_offline_queue[0].buf, *out_len);
  for (int i = 0; i < s_offline_queue_len - 1; i++) {
    s_offline_queue[i] = s_offline_queue[i + 1];
  }
  s_offline_queue_len--;
  return true;
}

static void serialize_contact(uint8_t out[CONTACT_FRAME_SIZE], const meshcore_contact_t *c) {
  uint16_t o = 0;
  memcpy(&out[o], c->pub_key, 32);
  o += 32;
  out[o++] = c->type;
  out[o++] = c->flags;
  out[o++] = c->out_path_len;
  memcpy(&out[o], c->out_path, MESHCORE_MAX_PATH);
  o += MESHCORE_MAX_PATH;
  memset(&out[o], 0, MESHCORE_NAME_MAX);
  strncpy((char *)&out[o], c->name, MESHCORE_NAME_MAX);
  o += MESHCORE_NAME_MAX;
  memcpy(&out[o], &c->last_advert, 4);
  o += 4;
  memcpy(&out[o], &c->gps_lat, 4);
  o += 4;
  memcpy(&out[o], &c->gps_lon, 4);
  o += 4;
}

static void deserialize_contact(meshcore_contact_t *c, const uint8_t in[], size_t in_len) {
  memset(c, 0, sizeof(*c));
  c->is_used = true;
  if (in_len < CONTACT_FRAME_SIZE)
    return;
  uint16_t o = 0;
  memcpy(c->pub_key, &in[o], 32);
  o += 32;
  c->type = in[o++];
  c->flags = in[o++];
  c->out_path_len = in[o++];
  memcpy(c->out_path, &in[o], MESHCORE_MAX_PATH);
  o += MESHCORE_MAX_PATH;
  char tmp[MESHCORE_NAME_MAX + 1] = {0};
  memcpy(tmp, &in[o], MESHCORE_NAME_MAX);
  strncpy(c->name, tmp, MESHCORE_NAME_MAX - 1);
  c->name[MESHCORE_NAME_MAX - 1] = 0;
  o += MESHCORE_NAME_MAX;
  memcpy(&c->last_advert, &in[o], 4);
  o += 4;
  memcpy(&c->gps_lat, &in[o], 4);
  o += 4;
  memcpy(&c->gps_lon, &in[o], 4);
  o += 4;
  if (in_len >= CONTACT_FRAME_SIZE + 4) {
    memcpy(&c->lastmod, &in[o], 4);
  }
}

static void write_contact_resp_frame(uint8_t code, const meshcore_contact_t *c) {
  uint8_t buf[1 + CONTACT_FRAME_SIZE + 4];
  buf[0] = code;
  serialize_contact(&buf[1], c);
  memcpy(&buf[1 + CONTACT_FRAME_SIZE], &c->lastmod, 4);
  send_resp(buf, sizeof(buf));
}

static void handle_device_query(const uint8_t *p, uint16_t len) {
  if (len >= 2) {
    s_app_target_ver = p[1];
    ESP_LOGI(TAG, "app_target_ver = %u", s_app_target_ver);
  }
  uint8_t resp[80];
  uint16_t o = 0;
  resp[o++] = RESP_CODE_DEVICE_INFO;
  resp[o++] = FIRMWARE_VER_CODE;
  resp[o++] = MESHCORE_MAX_CONTACTS / 2;
  resp[o++] = MESHCORE_MAX_CHANNELS;

  resp[o++] = (uint8_t)(s_ble_pin & 0xFF);
  resp[o++] = (uint8_t)((s_ble_pin >> 8) & 0xFF);
  resp[o++] = (uint8_t)((s_ble_pin >> 16) & 0xFF);
  resp[o++] = (uint8_t)((s_ble_pin >> 24) & 0xFF);

  const char *bd = __DATE__;
  uint8_t bl = (uint8_t)strlen(bd);
  if (bl > 12)
    bl = 12;
  memcpy(&resp[o], bd, bl);
  o += bl;
  while (bl++ < 12) {
    resp[o++] = 0;
  }

  const char *mo = DEVICE_MODEL;
  uint8_t ml = (uint8_t)strlen(mo);
  if (ml > 40)
    ml = 40;
  memcpy(&resp[o], mo, ml);
  o += ml;
  while (ml++ < 40) {
    resp[o++] = 0;
  }

  const char *ve = FIRMWARE_VERSION;
  uint8_t vl = (uint8_t)strlen(ve);
  if (vl > 20)
    vl = 20;
  memcpy(&resp[o], ve, vl);
  o += vl;
  while (vl++ < 20) {
    resp[o++] = 0;
  }

  resp[o++] = 0;
  resp[o++] = 0;

  ESP_LOGI(TAG, "-> DEVICE_INFO (%u bytes, target_ver=%u)", o, s_app_target_ver);
  send_resp(resp, o);
}

static void handle_app_start(const uint8_t *p, uint16_t len) {
  const meshcore_radio_prefs_t *rp = meshcore_get_radio_prefs();

  uint8_t resp[128];
  uint16_t o = 0;
  resp[o++] = RESP_CODE_SELF_INFO;
  resp[o++] = meshcore_get_adv_type();
  resp[o++] = (uint8_t)rp->tx_power_dbm;
  resp[o++] = 22;

  uint8_t pub[32] = {0};
  meshcore_get_pub_key(pub);
  memcpy(&resp[o], pub, 32);
  o += 32;

  int32_t lat = 0, lon = 0;
  bool is_has_latlon = false;
  meshcore_get_advert_latlon(&lat, &lon, &is_has_latlon);
  memcpy(&resp[o], &lat, 4);
  o += 4;
  memcpy(&resp[o], &lon, 4);
  o += 4;

  resp[o++] = 0;
  resp[o++] = 0;
  resp[o++] = 0;
  resp[o++] = 0;

  uint32_t freq_khz = rp->freq_hz / 1000;
  uint32_t bw_khz = rp->bw_hz / 1000;
  memcpy(&resp[o], &freq_khz, 4);
  o += 4;
  memcpy(&resp[o], &bw_khz, 4);
  o += 4;
  resp[o++] = rp->sf;
  resp[o++] = rp->cr;

  char nm[MESHCORE_NAME_MAX] = {0};
  meshcore_get_name(nm, sizeof(nm));
  uint8_t nl = (uint8_t)strlen(nm);
  if (o + nl >= sizeof(resp))
    nl = (uint8_t)(sizeof(resp) - o - 1);
  memcpy(&resp[o], nm, nl);
  o += nl;

  ESP_LOGI(TAG, "-> SELF_INFO (%u bytes, name='%s')", o, nm);
  send_resp(resp, o);
}

static void handle_batt_storage(const uint8_t *p, uint16_t len) {
  uint8_t resp[16];
  uint16_t o = 0;
  resp[o++] = RESP_CODE_BATT_AND_STORAGE;
  resp[o++] = 0x04;
  resp[o++] = 0x10;
  resp[o++] = 0x00;
  resp[o++] = 0x04;
  resp[o++] = 0;
  resp[o++] = 0;
  resp[o++] = 0x00;
  resp[o++] = 0x04;
  resp[o++] = 0;
  resp[o++] = 0;
  send_resp(resp, o);
}

static void handle_sync_next_msg(const uint8_t *p, uint16_t len) {
  uint8_t frame[MC_OFFLINE_FRAME_MAX];
  uint16_t flen = 0;
  if (offline_queue_pop(frame, &flen)) {
    ESP_LOGI(TAG, "-> SYNC msg (qlen=%u, %u bytes)", s_offline_queue_len, flen);
    send_resp(frame, flen);
    return;
  }
  uint8_t resp[1] = {RESP_CODE_NO_MORE_MESSAGES};
  send_resp(resp, 1);
}

static void handle_get_device_time(const uint8_t *p, uint16_t len) {
  uint32_t ts = meshcore_get_unix_time();
  uint8_t resp[5];
  resp[0] = RESP_CODE_CURR_TIME;
  memcpy(&resp[1], &ts, 4);
  send_resp(resp, sizeof(resp));
}

static void handle_set_device_time(const uint8_t *p, uint16_t len) {
  if (len >= 5) {
    uint32_t ts;
    memcpy(&ts, &p[1], 4);
    meshcore_set_unix_time(ts);
  }
  send_ok();
}

static void handle_send_self_advert(const uint8_t *p, uint16_t len) {
  meshcore_send_advert();
  send_ok();
}

static void handle_set_advert_name(const uint8_t *p, uint16_t len) {
  if (len < 2) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  char name[MESHCORE_NAME_MAX] = {0};
  uint16_t nl = len - 1;
  if (nl > MESHCORE_NAME_MAX - 1)
    nl = MESHCORE_NAME_MAX - 1;
  memcpy(name, &p[1], nl);
  name[nl] = 0;
  meshcore_set_node_name(name);
  send_ok();
}

static void handle_set_advert_latlon(const uint8_t *p, uint16_t len) {
  if (len < 1 + 8) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  int32_t lat, lon;
  memcpy(&lat, &p[1], 4);
  memcpy(&lon, &p[5], 4);
  bool has = (lat != 0 || lon != 0);
  meshcore_set_advert_latlon(lat, lon, has);
  send_ok();
}

static void handle_add_update_contact(const uint8_t *p, uint16_t len) {
  if (len < 1 + CONTACT_FRAME_SIZE) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  meshcore_contact_t c;
  deserialize_contact(&c, &p[1], len - 1);
  if (!meshcore_contact_upsert(&c)) {
    send_err(ERR_CODE_TABLE_FULL);
    return;
  }
  ESP_LOGI(TAG,
           "Contact ADD/UPDATE pubkey=%02X%02X%02X%02X type=%u",
           c.pub_key[0],
           c.pub_key[1],
           c.pub_key[2],
           c.pub_key[3],
           c.type);
  send_ok();
}

static void handle_get_contact_by_key(const uint8_t *p, uint16_t len) {
  if (len < 1 + 32) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  const meshcore_contact_t *c = meshcore_contact_find(&p[1]);
  if (c == NULL) {
    send_err(ERR_CODE_NOT_FOUND);
    return;
  }
  write_contact_resp_frame(RESP_CODE_CONTACT, c);
}

static void handle_get_contacts(const uint8_t *p, uint16_t len) {
  uint32_t since = 0;
  if (len >= 5)
    memcpy(&since, &p[1], 4);

  uint32_t count = 0;
  const meshcore_contact_t *arr = meshcore_contacts_array();
  for (int i = 0; i < MESHCORE_MAX_CONTACTS; i++) {
    if (arr[i].is_used && arr[i].lastmod >= since)
      count++;
  }

  uint8_t start[5];
  start[0] = RESP_CODE_CONTACTS_START;
  memcpy(&start[1], &count, 4);
  send_resp(start, 5);

  uint32_t max_lastmod = 0;
  for (int i = 0; i < MESHCORE_MAX_CONTACTS; i++) {
    if (arr[i].is_used && arr[i].lastmod >= since) {
      write_contact_resp_frame(RESP_CODE_CONTACT, &arr[i]);
      if (arr[i].lastmod > max_lastmod)
        max_lastmod = arr[i].lastmod;
    }
  }

  uint8_t end[5] = {RESP_CODE_END_OF_CONTACTS, 0, 0, 0, 0};
  memcpy(&end[1], &max_lastmod, 4);
  send_resp(end, 5);
  ESP_LOGI(TAG, "-> CONTACTS (%lu entries since=%lu)", (unsigned long)count, (unsigned long)since);
}

static void handle_remove_contact(const uint8_t *p, uint16_t len) {
  if (len < 1 + 32) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  if (!meshcore_contact_remove(&p[1])) {
    send_err(ERR_CODE_NOT_FOUND);
    return;
  }
  send_ok();
}

static void handle_share_contact(const uint8_t *p, uint16_t len) {
  meshcore_send_advert();
  send_ok();
}

static void handle_export_contact(const uint8_t *p, uint16_t len) {
  if (len < 1 + 32) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  const meshcore_contact_t *c = meshcore_contact_find(&p[1]);
  if (c == NULL) {
    send_err(ERR_CODE_NOT_FOUND);
    return;
  }
  uint8_t resp[1 + 32];
  resp[0] = RESP_CODE_EXPORT_CONTACT;
  memcpy(&resp[1], c->pub_key, 32);
  send_resp(resp, sizeof(resp));
}

static void handle_import_contact(const uint8_t *p, uint16_t len) {
  if (len < 1 + 32 + 2) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  meshcore_contact_t c;
  memset(&c, 0, sizeof(c));
  c.is_used = true;
  memcpy(c.pub_key, &p[1], 32);
  c.type = p[33];
  c.flags = p[34];
  c.out_path_len = MESHCORE_OUT_PATH_UNKNOWN;
  c.lastmod = meshcore_get_unix_time();
  if (!meshcore_contact_upsert(&c)) {
    send_err(ERR_CODE_TABLE_FULL);
    return;
  }
  send_ok();
}

static void handle_reset_path(const uint8_t *p, uint16_t len) {
  if (len < 1 + 32) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  if (!meshcore_contact_reset_path(&p[1])) {
    send_err(ERR_CODE_NOT_FOUND);
    return;
  }
  send_ok();
}

static void handle_get_channel(const uint8_t *p, uint16_t len) {
  uint8_t idx = (len >= 2) ? p[1] : 0;
  const meshcore_channel_t *ch = meshcore_channel_get(idx);
  if (ch == NULL) {
    send_err(ERR_CODE_NOT_FOUND);
    return;
  }
  uint8_t resp[2 + MESHCORE_NAME_MAX + 16];
  uint16_t o = 0;
  resp[o++] = RESP_CODE_CHANNEL_INFO;
  resp[o++] = idx;
  memset(&resp[o], 0, MESHCORE_NAME_MAX);
  strncpy((char *)&resp[o], ch->name, MESHCORE_NAME_MAX);
  o += MESHCORE_NAME_MAX;
  memcpy(&resp[o], ch->secret, 16);
  o += 16;
  send_resp(resp, o);
}

static void handle_set_channel(const uint8_t *p, uint16_t len) {
  if (len < 2 + MESHCORE_NAME_MAX + 16) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  uint8_t idx = p[1];
  char name[MESHCORE_NAME_MAX + 1] = {0};
  memcpy(name, &p[2], MESHCORE_NAME_MAX);
  if (!meshcore_channel_set(idx, name, &p[2 + MESHCORE_NAME_MAX])) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  send_ok();
}

static void handle_get_advert_path(const uint8_t *p, uint16_t len) {
  if (len < 2 + 32) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  const meshcore_contact_t *c = meshcore_contact_find(&p[2]);
  uint32_t ts = meshcore_get_unix_time();
  uint8_t resp[6 + MESHCORE_MAX_PATH];
  uint16_t o = 0;
  resp[o++] = RESP_CODE_ADVERT_PATH;
  memcpy(&resp[o], &ts, 4);
  o += 4;
  if (c == NULL || c->out_path_len == MESHCORE_OUT_PATH_UNKNOWN) {
    resp[o++] = 0;
  } else {
    resp[o++] = c->out_path_len;
    memcpy(&resp[o], c->out_path, c->out_path_len);
    o += c->out_path_len;
  }
  send_resp(resp, o);
}

static void handle_send_channel_txt(const uint8_t *p, uint16_t len) {
  if (len < 1 + 1 + 1 + 4) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  uint8_t channel_idx = p[2];
  uint16_t text_off = 1 + 1 + 1 + 4;
  uint16_t text_len = len - text_off;

  char text[160];
  if (text_len > sizeof(text) - 1)
    text_len = sizeof(text) - 1;
  memcpy(text, &p[text_off], text_len);
  text[text_len] = 0;

  esp_err_t ret = meshcore_send_grp_txt(channel_idx, text);
  if (ret != ESP_OK) {
    send_err(ERR_CODE_BAD_STATE);
    return;
  }

  uint8_t resp[1 + 1 + 4 + 4];
  uint16_t o = 0;
  resp[o++] = RESP_CODE_SENT;
  resp[o++] = 0;
  memset(&resp[o], 0, 4);
  o += 4;
  uint32_t timeout = 10000;
  memcpy(&resp[o], &timeout, 4);
  o += 4;
  send_resp(resp, o);
}

static void handle_send_txt_msg(const uint8_t *p, uint16_t len) {
  if (len < 1 + 1 + 1 + 4 + 32) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  uint8_t txt_type = p[1];
  uint8_t attempt = p[2];
  uint32_t ts;
  memcpy(&ts, &p[3], 4);
  const uint8_t *peer = &p[7];
  uint16_t text_off = 1 + 1 + 1 + 4 + 32;
  uint16_t text_len = len - text_off;
  char text[MESHCORE_TEXT_MAX + 1];
  if (text_len > sizeof(text) - 1)
    text_len = sizeof(text) - 1;
  memcpy(text, &p[text_off], text_len);
  text[text_len] = 0;
  (void)txt_type;

  uint32_t expected_ack = 0;
  esp_err_t ret = meshcore_send_direct_msg(peer, text, ts, attempt, &expected_ack);
  if (ret != ESP_OK) {
    send_err(ERR_CODE_BAD_STATE);
    return;
  }

  uint8_t resp[1 + 1 + 4 + 4];
  uint16_t o = 0;
  resp[o++] = RESP_CODE_SENT;
  resp[o++] = 1;
  memcpy(&resp[o], &expected_ack, 4);
  o += 4;
  uint32_t timeout = 10000;
  memcpy(&resp[o], &timeout, 4);
  o += 4;
  send_resp(resp, o);
  ESP_LOGI(TAG,
           "DM TX queued (peer=%02X%02X%02X%02X tag=0x%08lX)",
           peer[0],
           peer[1],
           peer[2],
           peer[3],
           (unsigned long)expected_ack);
}

static void handle_set_radio_params(const uint8_t *p, uint16_t len) {
  if (len < 1 + 4 + 4 + 1 + 1) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  uint32_t freq_khz, bw_khz;
  memcpy(&freq_khz, &p[1], 4);
  memcpy(&bw_khz, &p[5], 4);
  uint8_t sf = p[9];
  uint8_t cr = p[10];

  if (meshcore_set_radio_params(
          freq_khz * 1000UL, bw_khz * 1000UL, sf, cr, meshcore_get_radio_prefs()->tx_power_dbm) !=
      ESP_OK) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  send_ok();
}

static void handle_set_radio_tx_power(const uint8_t *p, uint16_t len) {
  if (len < 2) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  int8_t power = (int8_t)p[1];
  const meshcore_radio_prefs_t *rp = meshcore_get_radio_prefs();
  if (meshcore_set_radio_params(rp->freq_hz, rp->bw_hz, rp->sf, rp->cr, power) != ESP_OK) {
    send_err(ERR_CODE_ILLEGAL_ARG);
    return;
  }
  send_ok();
}

static int rx_char_access(uint16_t conn_handle,
                          uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt,
                          void *arg) {
  (void)arg;
  if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR)
    return 0;

  uint8_t buf[256];
  uint16_t len = 0;
  int rc = ble_hs_mbuf_to_flat(ctxt->om, buf, sizeof(buf), &len);
  if (rc != 0 || len == 0)
    return 0;

  ESP_LOGI(TAG, "CMD 0x%02X (%u bytes)", buf[0], len);

  switch (buf[0]) {
    case CMD_DEVICE_QUERY:
      handle_device_query(buf, len);
      break;
    case CMD_APP_START:
      handle_app_start(buf, len);
      break;
    case CMD_GET_BATT_AND_STORAGE:
      handle_batt_storage(buf, len);
      break;
    case CMD_SYNC_NEXT_MESSAGE:
      handle_sync_next_msg(buf, len);
      break;
    case CMD_GET_DEVICE_TIME:
      handle_get_device_time(buf, len);
      break;
    case CMD_SET_DEVICE_TIME:
      handle_set_device_time(buf, len);
      break;
    case CMD_SEND_SELF_ADVERT:
      handle_send_self_advert(buf, len);
      break;
    case CMD_SET_ADVERT_NAME:
      handle_set_advert_name(buf, len);
      break;
    case CMD_SET_ADVERT_LATLON:
      handle_set_advert_latlon(buf, len);
      break;
    case CMD_SEND_CHANNEL_TXT_MSG:
      handle_send_channel_txt(buf, len);
      break;
    case CMD_SEND_TXT_MSG:
      handle_send_txt_msg(buf, len);
      break;
    case CMD_GET_CONTACTS:
      handle_get_contacts(buf, len);
      break;
    case CMD_GET_CHANNEL:
      handle_get_channel(buf, len);
      break;
    case CMD_SET_CHANNEL:
      handle_set_channel(buf, len);
      break;
    case CMD_GET_ADVERT_PATH:
      handle_get_advert_path(buf, len);
      break;
    case CMD_ADD_UPDATE_CONTACT:
      handle_add_update_contact(buf, len);
      break;
    case CMD_GET_CONTACT_BY_KEY:
      handle_get_contact_by_key(buf, len);
      break;
    case CMD_REMOVE_CONTACT:
      handle_remove_contact(buf, len);
      break;
    case CMD_SHARE_CONTACT:
      handle_share_contact(buf, len);
      break;
    case CMD_EXPORT_CONTACT:
      handle_export_contact(buf, len);
      break;
    case CMD_IMPORT_CONTACT:
      handle_import_contact(buf, len);
      break;
    case CMD_RESET_PATH:
      handle_reset_path(buf, len);
      break;
    case CMD_SET_RADIO_PARAMS:
      handle_set_radio_params(buf, len);
      break;
    case CMD_SET_RADIO_TX_POWER:
      handle_set_radio_tx_power(buf, len);
      break;

    case CMD_SET_TUNING_PARAMS:
    case CMD_SET_FLOOD_SCOPE:
      send_ok();
      break;

    default:
      ESP_LOGW(TAG, "Unhandled CMD 0x%02X", buf[0]);
      send_ok();
      break;
  }
  return 0;
}

static int tx_char_access(uint16_t conn_handle,
                          uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt,
                          void *arg) {
  (void)conn_handle;
  (void)attr_handle;
  (void)ctxt;
  (void)arg;
  return 0;
}

static void ble_advertise(void) {
  struct ble_gap_adv_params adv_params = {0};
  struct ble_hs_adv_fields fields = {0};

  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  fields.tx_pwr_lvl_is_present = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
  fields.name = (uint8_t *)s_device_name;
  fields.name_len = strlen(s_device_name);
  fields.name_is_complete = 1;

  int rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "adv_set_fields failed: %d", rc);
    return;
  }

  struct ble_hs_adv_fields rsp_fields = {0};
  rsp_fields.uuids128 = (ble_uuid128_t *)&NUS_SERVICE_UUID;
  rsp_fields.num_uuids128 = 1;
  rsp_fields.uuids128_is_complete = 1;
  ble_gap_adv_rsp_set_fields(&rsp_fields);

  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

  rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event_cb, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "adv_start failed: %d", rc);
    return;
  }
  ESP_LOGI(TAG, "Advertising '%s' (PIN %lu)", s_device_name, (unsigned long)s_ble_pin);
}

static int gap_event_cb(struct ble_gap_event *event, void *arg) {
  (void)arg;
  switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
      if (event->connect.status == 0) {
        s_conn_handle = event->connect.conn_handle;
        ESP_LOGI(TAG, "Client connected (handle=%u)", s_conn_handle);
        ble_gattc_exchange_mtu(s_conn_handle, NULL, NULL);
      } else {
        ESP_LOGW(TAG, "Connect failed (status=%d)", event->connect.status);
        ble_advertise();
      }
      break;

    case BLE_GAP_EVENT_DISCONNECT:
      ESP_LOGI(TAG, "Client disconnected (reason=%d)", event->disconnect.reason);
      s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
      s_app_target_ver = 0;
      s_offline_queue_len = 0;
      ble_advertise();
      break;

    case BLE_GAP_EVENT_SUBSCRIBE:
      ESP_LOGI(TAG,
               "Subscribe attr=%u notify=%d",
               event->subscribe.attr_handle,
               event->subscribe.cur_notify);
      break;

    case BLE_GAP_EVENT_MTU:
      ESP_LOGI(TAG, "MTU update: %u", event->mtu.value);
      break;

    case BLE_GAP_EVENT_ENC_CHANGE:
      ESP_LOGI(TAG, "Encryption change: status=%d", event->enc_change.status);
      break;

    case BLE_GAP_EVENT_REPEAT_PAIRING: {
      struct ble_gap_conn_desc desc;
      if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == 0) {
        ble_store_util_delete_peer(&desc.peer_id_addr);
        ESP_LOGI(TAG, "Repeat pairing: deleted old bond");
      }
      return BLE_GAP_REPEAT_PAIRING_RETRY;
    }

    case BLE_GAP_EVENT_PASSKEY_ACTION:
      if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
        struct ble_sm_io pkey = {0};
        pkey.action = BLE_SM_IOACT_DISP;
        pkey.passkey = s_ble_pin;
        ble_sm_inject_io(event->passkey.conn_handle, &pkey);
        ESP_LOGI(TAG, "Displaying passkey: %lu", (unsigned long)s_ble_pin);
      }
      break;

    default:
      break;
  }
  return 0;
}

static void on_sync(void) {
  int rc = ble_hs_util_ensure_addr(0);
  if (rc != 0) {
    ESP_LOGE(TAG, "ensure_addr failed: %d", rc);
    return;
  }
  rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
  if (rc != 0) {
    ESP_LOGE(TAG, "infer_auto failed: %d", rc);
    return;
  }
  s_is_ble_ready = true;
  ble_advertise();
}

static void on_reset(int reason) {
  ESP_LOGW(TAG, "BLE reset, reason=%d", reason);
  s_is_ble_ready = false;
}

static void host_task(void *param) {
  (void)param;
  nimble_port_run();
  nimble_port_freertos_deinit();
}
