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

#include "bridge_manager.h"

#include <string.h>

#include "esp_log.h"

#include "c5_flasher.h"
#include "ota_version.h"
#include "spi_bridge.h"

static const char *TAG = "BRIDGE_MGR";

#define VERSION_BUF_SIZE   32
#define VERSION_TIMEOUT_MS 1000

esp_err_t bridge_manager_init(void) {
  ESP_LOGI(TAG, "Initializing bridge manager");
  ESP_LOGI(TAG, "Expected C5 version: %s", FIRMWARE_VERSION);

  if (spi_bridge_master_init() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init SPI bridge");
    return ESP_FAIL;
  }

  spi_header_t resp_header;
  uint8_t resp_ver[VERSION_BUF_SIZE];
  memset(resp_ver, 0, sizeof(resp_ver));

  ESP_LOGI(TAG, "Checking C5 version");
  esp_err_t ret = spi_bridge_send_command(
      SPI_ID_SYSTEM_VERSION, NULL, 0, &resp_header, resp_ver, VERSION_TIMEOUT_MS);

  bool is_update_needed = false;

  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "C5 not responding, assuming recovery needed");
    is_update_needed = true;
  } else {
    ESP_LOGI(TAG, "C5 version: %s (expected: %s)", resp_ver, FIRMWARE_VERSION);
    if (strcmp((char *)resp_ver, FIRMWARE_VERSION) != 0) {
      is_update_needed = true;
    }
  }

  if (is_update_needed) {
    ESP_LOGW(TAG, "C5 update required");
    c5_flasher_init();
    if (c5_flasher_update(NULL, 0) == ESP_OK) {
      ESP_LOGI(TAG, "C5 firmware uploaded");
    } else {
      ESP_LOGE(TAG, "C5 synchronization failed");
      spi_bridge_set_alive(false);
      return ESP_FAIL;
    }
  } else {
    ESP_LOGI(TAG, "C5 is up to date");
  }

  // Final probe: confirm the SPI bridge is actually responding before
  // letting background tasks poll it
  memset(resp_ver, 0, sizeof(resp_ver));
  ret = spi_bridge_send_command(
      SPI_ID_SYSTEM_VERSION, NULL, 0, &resp_header, resp_ver, VERSION_TIMEOUT_MS);
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "C5 SPI bridge not responding — disabling bridge polling");
    spi_bridge_set_alive(false);
    return ESP_OK;
  }

  ESP_LOGI(TAG, "C5 bridge alive");
  return ESP_OK;
}

esp_err_t bridge_manager_force_update(void) {
  c5_flasher_init();
  return c5_flasher_update(NULL, 0);
}
