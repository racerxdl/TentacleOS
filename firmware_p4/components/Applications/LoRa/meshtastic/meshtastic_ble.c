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

#include "meshtastic_ble.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
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
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "meshtastic_phoneapi.h"

extern void ble_store_config_init(void);

static const char *TAG = "MESHTASTIC_BLE";

#define MT_BLE_PREFERRED_MTU    512
#define MT_BLE_FROMRADIO_BUFSZ  512
#define MT_BLE_PASSKEY_MIN      100000
#define MT_BLE_PASSKEY_RANGE    900000
#define MT_BLE_NOTIFY_TICK_MS   100
#define MT_BLE_NOTIFY_TASK_SZ   3072
#define MT_BLE_BATTERY_STUB_PCT 87

static const ble_uuid128_t MESH_SERVICE_UUID = BLE_UUID128_INIT(
    0xfd, 0xea, 0x73, 0xe2, 0xca, 0x5d, 0xa8, 0x9f,
    0x1f, 0x46, 0xa8, 0x15, 0x18, 0xb2, 0xa1, 0x6b);
static const ble_uuid128_t FROMRADIO_UUID = BLE_UUID128_INIT(
    0x02, 0x00, 0x12, 0xac, 0x42, 0x02, 0x78, 0xb8,
    0xed, 0x11, 0x93, 0x49, 0x9e, 0xe6, 0x55, 0x2c);
static const ble_uuid128_t TORADIO_UUID = BLE_UUID128_INIT(
    0xe7, 0x01, 0x44, 0x12, 0x66, 0x78, 0xdd, 0xa1,
    0xad, 0x4d, 0x9e, 0x12, 0xd2, 0x76, 0x5c, 0xf7);
static const ble_uuid128_t FROMNUM_UUID = BLE_UUID128_INIT(
    0x53, 0x44, 0xe3, 0x47, 0x75, 0xaa, 0x70, 0xa6,
    0x66, 0x4f, 0x00, 0xa8, 0x8c, 0xa1, 0x9d, 0xed);
static const ble_uuid128_t LOGRADIO_UUID = BLE_UUID128_INIT(
    0x47, 0x95, 0xdf, 0x8c, 0xde, 0xe9, 0x44, 0x99,
    0x23, 0x44, 0xe6, 0x06, 0x49, 0x6e, 0x3d, 0x5a);

static const ble_uuid16_t BATTERY_SERVICE_UUID = BLE_UUID16_INIT(0x180F);
static const ble_uuid16_t BATTERY_LEVEL_UUID   = BLE_UUID16_INIT(0x2A19);

static uint32_t s_node_num = 0;
static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static bool     s_is_connected = false;
static bool     s_fromnum_subscribed = false;
static uint16_t s_fromnum_handle = 0;
static uint32_t s_fromnum_counter = 0;
static uint8_t  s_own_addr_type = 0;
static uint32_t s_random_passkey = 0;
static TaskHandle_t s_notify_task = NULL;

#define MT_BLE_LOG_RING_SIZE 2048
#define MT_BLE_LOG_CHUNK_MAX 200

static uint16_t    s_logradio_handle = 0;
static bool        s_logradio_subscribed = false;
static uint8_t     s_log_ring[MT_BLE_LOG_RING_SIZE];
static uint16_t    s_log_head = 0;
static uint16_t    s_log_tail = 0;
static vprintf_like_t s_prev_vprintf = NULL;

static void log_ring_push(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        uint16_t next = (s_log_tail + 1) % MT_BLE_LOG_RING_SIZE;
        if (next == s_log_head) {
            s_log_head = (s_log_head + 1) % MT_BLE_LOG_RING_SIZE;
        }
        s_log_ring[s_log_tail] = (uint8_t)buf[i];
        s_log_tail = next;
    }
}

static uint16_t log_ring_pop(uint8_t *out, uint16_t max_len)
{
    uint16_t n = 0;
    while (n < max_len && s_log_head != s_log_tail) {
        out[n++] = s_log_ring[s_log_head];
        s_log_head = (s_log_head + 1) % MT_BLE_LOG_RING_SIZE;
    }
    return n;
}

static int log_vprintf(const char *fmt, va_list args)
{
    char line[256];
    int len = vsnprintf(line, sizeof(line), fmt, args);
    if (len > 0) {
        if (len >= (int)sizeof(line)) len = sizeof(line) - 1;
        log_ring_push(line, (size_t)len);
    }
    if (s_prev_vprintf != NULL) {
        va_list copy;
        va_copy(copy, args);
        int r = s_prev_vprintf(fmt, copy);
        va_end(copy);
        return r;
    }
    return len;
}

static void notify_logradio(void)
{
    if (!s_is_connected || !s_logradio_subscribed || s_logradio_handle == 0) return;
    if (s_log_head == s_log_tail) return;
    uint8_t chunk[MT_BLE_LOG_CHUNK_MAX];
    uint16_t n = log_ring_pop(chunk, sizeof(chunk));
    if (n == 0) return;
    struct os_mbuf *om = ble_hs_mbuf_from_flat(chunk, n);
    if (om != NULL) {
        ble_gatts_notify_custom(s_conn_handle, s_logradio_handle, om);
    }
}

static uint8_t s_battery_level = MT_BLE_BATTERY_STUB_PCT;

static void advertise_start(void);

static void notify_fromnum(void)
{
    if (!s_is_connected || !s_fromnum_subscribed || s_fromnum_handle == 0) return;
    s_fromnum_counter++;
    struct os_mbuf *om = ble_hs_mbuf_from_flat(&s_fromnum_counter,
                                                 sizeof(s_fromnum_counter));
    if (om != NULL) {
        ble_gatts_notify_custom(s_conn_handle, s_fromnum_handle, om);
    }
}

static void notify_task(void *arg)
{
    (void)arg;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(MT_BLE_NOTIFY_TICK_MS));
        if (!s_is_connected) continue;
        if (phoneapi_has_data()) {
            notify_fromnum();
        }
        notify_logradio();
    }
}

static uint8_t s_fromradio_buf[MT_BLE_FROMRADIO_BUFSZ];
static uint8_t s_toradio_buf[MT_BLE_FROMRADIO_BUFSZ];

static int on_fromradio_access(uint16_t conn, uint16_t attr,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn; (void)attr; (void)arg;
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) return BLE_ATT_ERR_UNLIKELY;

    uint16_t flen = phoneapi_poll_fromradio(s_fromradio_buf, sizeof(s_fromradio_buf));
    if (flen > 0) {
        os_mbuf_append(ctxt->om, s_fromradio_buf, flen);
        ESP_LOGD(TAG, "FROMRADIO read %u bytes", flen);
    }
    return 0;
}

static int on_toradio_access(uint16_t conn, uint16_t attr,
                              struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn; (void)attr; (void)arg;
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) return BLE_ATT_ERR_UNLIKELY;

    uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
    if (len == 0 || len > MT_BLE_FROMRADIO_BUFSZ) return 0;
    os_mbuf_copydata(ctxt->om, 0, len, s_toradio_buf);

    ESP_LOGI(TAG, "TORADIO write %u bytes", len);
    phoneapi_on_toradio(s_toradio_buf, len);
    if (phoneapi_has_data()) notify_fromnum();
    return 0;
}

static int on_fromnum_access(uint16_t conn, uint16_t attr,
                              struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn; (void)attr; (void)arg;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        os_mbuf_append(ctxt->om, &s_fromnum_counter, sizeof(s_fromnum_counter));
    }
    return 0;
}

static int on_logradio_access(uint16_t conn, uint16_t attr,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn; (void)attr; (void)arg; (void)ctxt;
    return 0;
}

static int on_battery_level_access(uint16_t conn, uint16_t attr,
                                    struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn; (void)attr; (void)arg;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        os_mbuf_append(ctxt->om, &s_battery_level, 1);
    }
    return 0;
}

static const struct ble_gatt_svc_def GATT_SERVICES[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &MESH_SERVICE_UUID.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &TORADIO_UUID.u,
                .access_cb = on_toradio_access,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {
                .uuid = &FROMRADIO_UUID.u,
                .access_cb = on_fromradio_access,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                .uuid = &FROMNUM_UUID.u,
                .access_cb = on_fromnum_access,
                .val_handle = &s_fromnum_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            {
                .uuid = &LOGRADIO_UUID.u,
                .access_cb = on_logradio_access,
                .val_handle = &s_logradio_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            { 0 },
        },
    },
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &BATTERY_SERVICE_UUID.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &BATTERY_LEVEL_UUID.u,
                .access_cb = on_battery_level_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            { 0 },
        },
    },
    { 0 },
};

static int gap_event(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    switch (event->type) {

    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            s_conn_handle = event->connect.conn_handle;
            s_is_connected = true;
            s_fromnum_subscribed = false;
            s_fromnum_counter = 0;
            ESP_LOGI(TAG, "Client connected handle=%u", s_conn_handle);
            ble_att_set_preferred_mtu(MT_BLE_PREFERRED_MTU);
        } else {
            ESP_LOGW(TAG, "Connection failed status=%d", event->connect.status);
            advertise_start();
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Client disconnected reason=0x%x", event->disconnect.reason);
        s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_is_connected = false;
        s_fromnum_subscribed = false;
        phoneapi_disconnect();
        advertise_start();
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "MTU updated to %u", event->mtu.value);
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "Subscribe attr=%u notify=%u",
                 event->subscribe.attr_handle, event->subscribe.cur_notify);
        if (event->subscribe.attr_handle == s_fromnum_handle) {
            s_fromnum_subscribed = (event->subscribe.cur_notify != 0);
        }
        if (event->subscribe.attr_handle == s_logradio_handle) {
            s_logradio_subscribed = (event->subscribe.cur_notify != 0);
        }
        break;

    case BLE_GAP_EVENT_ENC_CHANGE:
        ESP_LOGI(TAG, "Encryption status=%d", event->enc_change.status);
        break;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            s_random_passkey = MT_BLE_PASSKEY_MIN + (esp_random() % MT_BLE_PASSKEY_RANGE);
            struct ble_sm_io pk = {
                .action = BLE_SM_IOACT_DISP,
                .passkey = s_random_passkey,
            };
            ble_sm_inject_io(event->passkey.conn_handle, &pk);
            ESP_LOGI(TAG, "========================================");
            ESP_LOGI(TAG, "  BLE pairing PIN: %lu", (unsigned long)s_random_passkey);
            ESP_LOGI(TAG, "========================================");
        }
        break;

    case BLE_GAP_EVENT_REPEAT_PAIRING: {
        struct ble_gap_conn_desc desc;
        if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) != 0) {
            return BLE_GAP_REPEAT_PAIRING_IGNORE;
        }
        ble_store_util_delete_peer(&desc.peer_id_addr);
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }

    default:
        break;
    }
    return 0;
}

static void advertise_start(void)
{
    struct ble_gap_adv_params adv = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
    };

    struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.uuids128 = (ble_uuid128_t *)&MESH_SERVICE_UUID;
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    struct ble_hs_adv_fields rsp = {0};
    rsp.name = (uint8_t *)ble_svc_gap_device_name();
    rsp.name_len = strlen(ble_svc_gap_device_name());
    rsp.name_is_complete = 1;
    ble_gap_adv_rsp_set_fields(&rsp);

    int rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER,
                                &adv, gap_event, NULL);
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        ESP_LOGE(TAG, "adv_start failed rc=%d", rc);
        return;
    }
    ESP_LOGI(TAG, "Advertising as '%s'", ble_svc_gap_device_name());
}

static void on_sync(void)
{
    ble_hs_util_ensure_addr(0);
    ble_hs_id_infer_auto(0, &s_own_addr_type);

    uint8_t mac[6];
    ble_hs_id_copy_addr(s_own_addr_type, mac, NULL);
    ESP_LOGI(TAG, "BLE addr %02x:%02x:%02x:%02x:%02x:%02x type=%u",
             mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], s_own_addr_type);

    advertise_start();
}

static void on_reset(int reason)
{
    ESP_LOGW(TAG, "NimBLE reset reason=%d", reason);
}

static void nimble_host_task(void *param)
{
    (void)param;
    ESP_LOGI(TAG, "NimBLE host task running");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t meshtastic_ble_init(uint32_t node_num)
{
    s_node_num = node_num;

    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nimble_port_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ble_hs_cfg.sm_io_cap  = BLE_SM_IO_CAP_DISP_ONLY;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm    = 1;
    ble_hs_cfg.sm_sc      = 1;
    ble_hs_cfg.sm_our_key_dist   = BLE_SM_PAIR_KEY_DIST_ENC |
                                   BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC |
                                   BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb  = on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    char name[32];
    snprintf(name, sizeof(name), "Highboy_%04lx",
             (unsigned long)(node_num & 0xFFFF));
    ble_svc_gap_device_name_set(name);

    ble_svc_gap_init();
    ble_svc_gatt_init();

    int rc = ble_gatts_count_cfg(GATT_SERVICES);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_count_cfg failed rc=%d", rc);
        return ESP_FAIL;
    }
    rc = ble_gatts_add_svcs(GATT_SERVICES);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_add_svcs failed rc=%d", rc);
        return ESP_FAIL;
    }

    ble_store_config_init();
    nimble_port_freertos_init(nimble_host_task);

    xTaskCreate(notify_task, "ble_notify", MT_BLE_NOTIFY_TASK_SZ, NULL,
                tskIDLE_PRIORITY + 2, &s_notify_task);

    s_prev_vprintf = esp_log_set_vprintf(log_vprintf);

    ESP_LOGI(TAG, "Initialized - nome='%s'", name);
    return ESP_OK;
}

esp_err_t meshtastic_ble_start(void)
{
    advertise_start();
    return ESP_OK;
}

void meshtastic_ble_stop(void)
{
    ble_gap_adv_stop();
}

bool meshtastic_ble_is_connected(void)
{
    return s_is_connected;
}
