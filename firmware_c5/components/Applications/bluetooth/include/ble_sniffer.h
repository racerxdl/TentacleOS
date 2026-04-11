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

#ifndef BLE_SNIFFER_H
#define BLE_SNIFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

/**
 * @brief Start the BLE packet sniffer.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_NO_MEM if allocation fails
 */
esp_err_t ble_sniffer_start(void);

/**
 * @brief Stop the BLE packet sniffer and free resources.
 */
void ble_sniffer_stop(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_SNIFFER_H
