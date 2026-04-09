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

#include "gatt_explorer.h"

#include <string.h>

#include "esp_log.h"

#include "bluetooth_service.h"
#include "spi_bridge.h"
#include "spi_protocol.h"

static const char *TAG = "GATT_EXPLORER";

#define GATT_ADDR_PAYLOAD_SIZE 7
#define GATT_SPI_TIMEOUT_MS    10000

static bool s_is_busy = false;

bool gatt_explorer_start(const uint8_t *addr, uint8_t addr_type) {
  if (s_is_busy)
    return false;

  if (!bluetooth_service_is_running()) {
    ESP_LOGE(TAG, "Bluetooth service not running");
    return false;
  }

  uint8_t payload[GATT_ADDR_PAYLOAD_SIZE];
  memcpy(payload, addr, 6);
  payload[6] = addr_type;

  spi_header_t resp_hdr;
  uint8_t resp_buf[SPI_MAX_PAYLOAD];

  esp_err_t ret = spi_bridge_send_command(
      SPI_ID_BT_APP_GATT_EXP, payload, sizeof(payload), &resp_hdr, resp_buf, GATT_SPI_TIMEOUT_MS);

  if (ret != ESP_OK || resp_buf[0] != SPI_STATUS_OK) {
    ESP_LOGE(TAG, "Failed to start GATT exploration on C5");
    return false;
  }

  s_is_busy = true;
  ESP_LOGI(TAG, "GATT exploration started on C5");
  return true;
}

bool gatt_explorer_is_busy(void) {
  return s_is_busy;
}
