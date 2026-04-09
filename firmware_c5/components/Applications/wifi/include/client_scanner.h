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

#ifndef CLIENT_SCANNER_H
#define CLIENT_SCANNER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Record of a detected Wi-Fi client.
 */
typedef struct {
  uint8_t bssid[6];
  uint8_t client_mac[6];
  int8_t rssi;
  uint8_t channel;
} client_scanner_record_t;

/**
 * @brief Start a client scan across all channels.
 *
 * @return true on success, false if a scan is already in progress.
 */
bool client_scanner_start(void);

/**
 * @brief Get the scan results.
 *
 * @param[out] out_count  Number of clients found.
 * @return Pointer to result array, or NULL if scan is in progress.
 */
client_scanner_record_t *client_scanner_get_results(uint16_t *out_count);

/**
 * @brief Free scan results and associated task memory.
 */
void client_scanner_free_results(void);

/**
 * @brief Save scan results to internal flash storage.
 *
 * @return true on success, false on failure.
 */
bool client_scanner_save_results_to_internal_flash(void);

/**
 * @brief Save scan results to SD card.
 *
 * @return true on success, false on failure.
 */
bool client_scanner_save_results_to_sd_card(void);

#ifdef __cplusplus
}
#endif

#endif // CLIENT_SCANNER_H
