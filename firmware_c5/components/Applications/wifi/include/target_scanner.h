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

#ifndef TARGET_SCANNER_H
#define TARGET_SCANNER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Record of a client connected to the target AP.
 */
typedef struct {
  uint8_t client_mac[6];
  int8_t rssi;
} target_scanner_record_t;

/**
 * @brief Start a targeted scan for clients of a specific AP.
 *
 * @param target_bssid  BSSID of the target access point.
 * @param channel       Wi-Fi channel of the target.
 * @return true on success, false if a scan is already in progress.
 */
bool target_scanner_start(const uint8_t *target_bssid, uint8_t channel);

/**
 * @brief Get the scan results after scan completion.
 *
 * @param[out] out_count  Number of clients found.
 * @return Pointer to result array, or NULL if scan is in progress.
 */
target_scanner_record_t *target_scanner_get_results(uint16_t *out_count);

/**
 * @brief Get live results while scan is still running.
 *
 * @param[out] out_count     Number of clients found so far.
 * @param[out] out_scanning  Whether the scan is still in progress.
 * @return Pointer to result array.
 */
target_scanner_record_t *target_scanner_get_live_results(uint16_t *out_count, bool *out_scanning);

/**
 * @brief Get a pointer to the live result count.
 *
 * @return Pointer to the internal count variable.
 */
const uint16_t *target_scanner_get_count_ptr(void);

/**
 * @brief Check if a targeted scan is in progress.
 *
 * @return true if scanning, false otherwise.
 */
bool target_scanner_is_scanning(void);

/**
 * @brief Free scan results and associated task memory.
 */
void target_scanner_free_results(void);

/**
 * @brief Save scan results to internal flash storage.
 *
 * @return true on success, false on failure.
 */
bool target_scanner_save_results_to_internal_flash(void);

/**
 * @brief Save scan results to SD card.
 *
 * @return true on success, false on failure.
 */
bool target_scanner_save_results_to_sd_card(void);

#ifdef __cplusplus
}
#endif

#endif // TARGET_SCANNER_H
