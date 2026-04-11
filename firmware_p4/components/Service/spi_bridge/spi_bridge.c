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

#include "spi_bridge.h"

#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "spi_bridge_phy.h"

static const char *TAG = "SPI_BRIDGE_P4";

#define SPI_STREAM_TASK_STACK 4096
#define SPI_STREAM_TASK_PRIO  5
#define SPI_MUTEX_TIMEOUT_MS  50
#define SPI_STREAM_POLL_MS    10
#define SPI_STREAM_IDLE_MS    20
#define SPI_STREAM_YIELD_MS   1
#define SPI_IRQ_WAIT_MS       100

static SemaphoreHandle_t s_spi_mutex = NULL;
static TaskHandle_t s_stream_task_handle = NULL;
static volatile bool s_is_command_in_flight = false;
static spi_stream_cb_t s_stream_cb_wifi = NULL;
static spi_stream_cb_t s_stream_cb_bt = NULL;

static void stream_task(void *arg);
static esp_err_t fetch_stream(spi_header_t *out_header, uint8_t *out_payload, uint8_t *out_len);
static spi_stream_cb_t get_stream_cb(spi_id_t id);

static esp_err_t status_to_err(spi_status_t status) {
  switch (status) {
    case SPI_STATUS_OK:
      return ESP_OK;
    case SPI_STATUS_BUSY:
      return ESP_ERR_INVALID_STATE;
    case SPI_STATUS_UNSUPPORTED:
      return ESP_ERR_NOT_SUPPORTED;
    case SPI_STATUS_INVALID_ARG:
      return ESP_ERR_INVALID_ARG;
    case SPI_STATUS_ERROR:
    default:
      return ESP_FAIL;
  }
}

// Public functions

esp_err_t spi_bridge_master_init(void) {
  if (s_spi_mutex == NULL) {
    s_spi_mutex = xSemaphoreCreateMutex();
  }
  return spi_bridge_phy_init();
}

void spi_bridge_register_stream_cb(spi_id_t id, spi_stream_cb_t cb) {
  if (id == SPI_ID_WIFI_APP_SNIFFER)
    s_stream_cb_wifi = cb;
  if (id == SPI_ID_BT_APP_SNIFFER)
    s_stream_cb_bt = cb;

  if ((s_stream_cb_wifi != NULL || s_stream_cb_bt != NULL) && s_stream_task_handle == NULL) {
    if (s_spi_mutex == NULL) {
      s_spi_mutex = xSemaphoreCreateMutex();
    }
    BaseType_t created = xTaskCreate(stream_task,
                                     "spi_stream",
                                     SPI_STREAM_TASK_STACK,
                                     NULL,
                                     SPI_STREAM_TASK_PRIO,
                                     &s_stream_task_handle);
    if (created != pdPASS) {
      s_stream_task_handle = NULL;
      ESP_LOGE(TAG, "Failed to create SPI stream task");
    }
  }
}

void spi_bridge_unregister_stream_cb(spi_id_t id) {
  if (id == SPI_ID_WIFI_APP_SNIFFER)
    s_stream_cb_wifi = NULL;
  if (id == SPI_ID_BT_APP_SNIFFER)
    s_stream_cb_bt = NULL;
}

esp_err_t spi_bridge_send_command(spi_id_t id,
                                  const uint8_t *payload,
                                  uint8_t len,
                                  spi_header_t *out_header,
                                  uint8_t *out_payload,
                                  uint32_t timeout_ms) {
  if (s_spi_mutex == NULL) {
    s_spi_mutex = xSemaphoreCreateMutex();
  }
  if (xSemaphoreTake(s_spi_mutex, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
    return ESP_ERR_TIMEOUT;
  }
  s_is_command_in_flight = true;

  spi_header_t header = {.sync = SPI_SYNC_BYTE, .type = SPI_TYPE_CMD, .id = id, .length = len};

  if (len > SPI_MAX_PAYLOAD) {
    s_is_command_in_flight = false;
    xSemaphoreGive(s_spi_mutex);
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t tx_buf[SPI_FRAME_SIZE];
  uint8_t rx_buf[SPI_FRAME_SIZE];
  memset(tx_buf, 0, sizeof(tx_buf));
  memcpy(tx_buf, &header, sizeof(header));
  if (payload != NULL && len > 0) {
    memcpy(tx_buf + sizeof(header), payload, len);
  }

  esp_err_t ret = spi_bridge_phy_transmit(tx_buf, NULL, SPI_FRAME_SIZE);
  if (ret != ESP_OK) {
    s_is_command_in_flight = false;
    xSemaphoreGive(s_spi_mutex);
    return ret;
  }

  ret = spi_bridge_phy_wait_irq(timeout_ms);
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "Command 0x%02X timeout", id);
    s_is_command_in_flight = false;
    xSemaphoreGive(s_spi_mutex);
    return ret;
  }

  memset(tx_buf, 0, sizeof(tx_buf));
  memset(rx_buf, 0, sizeof(rx_buf));
  ret = spi_bridge_phy_transmit(tx_buf, rx_buf, SPI_FRAME_SIZE);
  if (ret != ESP_OK) {
    s_is_command_in_flight = false;
    xSemaphoreGive(s_spi_mutex);
    return ret;
  }

  spi_header_t *resp = (spi_header_t *)rx_buf;

  if (resp->sync != SPI_SYNC_BYTE) {
    ESP_LOGE(TAG, "Sync error: 0x%02X", resp->sync);
    s_is_command_in_flight = false;
    xSemaphoreGive(s_spi_mutex);
    return ESP_ERR_INVALID_RESPONSE;
  }

  if (resp->type != SPI_TYPE_RESP) {
    ESP_LOGE(TAG, "Invalid response type: 0x%02X", resp->type);
    s_is_command_in_flight = false;
    xSemaphoreGive(s_spi_mutex);
    return ESP_ERR_INVALID_RESPONSE;
  }

  if (resp->id != id) {
    ESP_LOGW(TAG, "Response ID mismatch (req 0x%02X, resp 0x%02X)", id, resp->id);
  }

  if (resp->length > SPI_MAX_PAYLOAD) {
    ESP_LOGE(TAG, "Invalid response length: %u", resp->length);
    s_is_command_in_flight = false;
    xSemaphoreGive(s_spi_mutex);
    return ESP_ERR_INVALID_RESPONSE;
  }

  spi_status_t status = SPI_STATUS_ERROR;
  uint8_t data_len = 0;
  if (resp->length >= SPI_RESP_STATUS_SIZE) {
    status = (spi_status_t)rx_buf[sizeof(spi_header_t)];
    data_len = (uint8_t)(resp->length - SPI_RESP_STATUS_SIZE);
  }

  if (out_header != NULL) {
    *out_header = *resp;
    out_header->length = data_len;
  }

  if (data_len > 0 && out_payload != NULL) {
    memcpy(out_payload, rx_buf + sizeof(spi_header_t) + SPI_RESP_STATUS_SIZE, data_len);
  }

  esp_err_t out = status_to_err(status);
  s_is_command_in_flight = false;
  xSemaphoreGive(s_spi_mutex);
  return out;
}

// Static functions

static spi_stream_cb_t get_stream_cb(spi_id_t id) {
  if (id == SPI_ID_WIFI_APP_SNIFFER)
    return s_stream_cb_wifi;
  if (id == SPI_ID_BT_APP_SNIFFER)
    return s_stream_cb_bt;
  return NULL;
}

static esp_err_t fetch_stream(spi_header_t *out_header, uint8_t *out_payload, uint8_t *out_len) {
  spi_header_t header = {
      .sync = SPI_SYNC_BYTE, .type = SPI_TYPE_CMD, .id = SPI_ID_SYSTEM_STREAM, .length = 0};

  uint8_t tx_buf[SPI_FRAME_SIZE];
  uint8_t rx_buf[SPI_FRAME_SIZE];
  memset(tx_buf, 0, sizeof(tx_buf));
  memcpy(tx_buf, &header, sizeof(header));

  esp_err_t ret = spi_bridge_phy_transmit(tx_buf, NULL, SPI_FRAME_SIZE);
  if (ret != ESP_OK)
    return ret;

  ret = spi_bridge_phy_wait_irq(SPI_IRQ_WAIT_MS);
  if (ret != ESP_OK)
    return ret;

  memset(tx_buf, 0, sizeof(tx_buf));
  memset(rx_buf, 0, sizeof(rx_buf));
  ret = spi_bridge_phy_transmit(tx_buf, rx_buf, SPI_FRAME_SIZE);
  if (ret != ESP_OK)
    return ret;

  spi_header_t *resp = (spi_header_t *)rx_buf;
  if (resp->sync != SPI_SYNC_BYTE)
    return ESP_ERR_INVALID_RESPONSE;

  if (resp->type == SPI_TYPE_STREAM) {
    if (out_header != NULL)
      *out_header = *resp;
    if (out_len != NULL)
      *out_len = resp->length;
    if (resp->length > 0 && out_payload != NULL) {
      memcpy(out_payload, rx_buf + sizeof(spi_header_t), resp->length);
    }
    return ESP_OK;
  }

  if (resp->type == SPI_TYPE_RESP && resp->length >= SPI_RESP_STATUS_SIZE) {
    spi_status_t status = (spi_status_t)rx_buf[sizeof(spi_header_t)];
    return status_to_err(status);
  }

  return ESP_ERR_INVALID_RESPONSE;
}

static void stream_task(void *arg) {
  uint8_t payload[SPI_MAX_PAYLOAD];
  spi_header_t header;

  while (1) {
    if (s_stream_cb_wifi == NULL && s_stream_cb_bt == NULL) {
      s_stream_task_handle = NULL;
      vTaskDelete(NULL);
      return;
    }

    if (s_is_command_in_flight) {
      vTaskDelay(pdMS_TO_TICKS(SPI_STREAM_POLL_MS));
      continue;
    }

    if (xSemaphoreTake(s_spi_mutex, pdMS_TO_TICKS(SPI_MUTEX_TIMEOUT_MS)) != pdTRUE) {
      vTaskDelay(pdMS_TO_TICKS(SPI_STREAM_POLL_MS));
      continue;
    }

    uint8_t len = 0;
    esp_err_t ret = fetch_stream(&header, payload, &len);
    xSemaphoreGive(s_spi_mutex);
    if (ret != ESP_OK) {
      vTaskDelay(pdMS_TO_TICKS(SPI_STREAM_IDLE_MS));
      continue;
    }

    spi_stream_cb_t cb = get_stream_cb(header.id);
    if (cb != NULL) {
      cb(header.id, payload, len);
    }
    vTaskDelay(pdMS_TO_TICKS(SPI_STREAM_YIELD_MS));
  }
}
