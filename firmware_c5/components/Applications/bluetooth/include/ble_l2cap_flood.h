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

#ifndef BLE_L2CAP_FLOOD_H
#define BLE_L2CAP_FLOOD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

/**
 * @brief Start L2CAP parameter flood attack against a target device.
 *
 * @param addr      Target device BLE address (6 bytes).
 * @param addr_type Target device address type.
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_STATE if already running
 *   - ESP_FAIL if bluetooth service is not running
 *   - ESP_ERR_NO_MEM if allocation fails
 */
esp_err_t ble_l2cap_flood_start(const uint8_t *addr, uint8_t addr_type);

/**
 * @brief Stop the L2CAP flood attack.
 *
 * @return
 *   - ESP_OK on success
 */
esp_err_t ble_l2cap_flood_stop(void);

/**
 * @brief Check if the L2CAP flood is currently running.
 *
 * @return true if running, false otherwise.
 */
bool ble_l2cap_flood_is_running(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_L2CAP_FLOOD_H
