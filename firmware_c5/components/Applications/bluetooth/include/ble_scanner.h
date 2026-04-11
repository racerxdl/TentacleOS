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

#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "bluetooth_service.h"

/**
 * @brief Save scan results to internal flash storage.
 *
 * @return true on success, false on failure.
 */
bool ble_scanner_save_results_to_internal_flash(void);

/**
 * @brief Save scan results to SD card.
 *
 * @return true on success, false on failure.
 */
bool ble_scanner_save_results_to_sd_card(void);

/**
 * @brief Start a BLE scan task.
 *
 * @return true if scan started, false on failure.
 */
bool ble_scanner_start(void);

/**
 * @brief Get the scan results.
 *
 * @param[out] out_count Pointer to store the number of results.
 * @return Pointer to the scan results array, or NULL if scanning or empty.
 */
bluetooth_service_scan_result_t *ble_scanner_get_results(uint16_t *out_count);

/**
 * @brief Free the scan results and release associated memory.
 */
void ble_scanner_free_results(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_SCANNER_H
