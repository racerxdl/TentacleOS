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

#ifndef BLE_TRACKER_H
#define BLE_TRACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "esp_err.h"

/**
 * @brief Start tracking a BLE device by address, monitoring its RSSI.
 *
 * @param addr Target device BLE address (6 bytes).
 * @return
 *   - ESP_OK on success
 */
esp_err_t ble_tracker_start(const uint8_t *addr);

/**
 * @brief Stop tracking the BLE device.
 */
void ble_tracker_stop(void);

/**
 * @brief Get the latest RSSI value from the tracked device.
 *
 * @return RSSI value in dBm, or -120 if no reading available.
 */
int ble_tracker_get_rssi(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_TRACKER_H
