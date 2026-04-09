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

#ifndef PROBE_MONITOR_H
#define PROBE_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Record of a captured probe request.
 */
typedef struct {
  uint8_t mac[6];
  int8_t rssi;
  char ssid[33];
  uint32_t last_seen_timestamp;
} probe_monitor_record_t;

/**
 * @brief Start the probe request monitor.
 *
 * @return true on success, false on failure.
 */
bool probe_monitor_start(void);

/**
 * @brief Stop the probe request monitor.
 */
void probe_monitor_stop(void);

/**
 * @brief Get the captured probe request results.
 *
 * @param[out] out_count  Number of probe records found.
 * @return Pointer to result array.
 */
probe_monitor_record_t *probe_monitor_get_results(uint16_t *out_count);

/**
 * @brief Get a pointer to the live result count.
 *
 * @return Pointer to the internal count variable.
 */
const uint16_t *probe_monitor_get_count_ptr(void);

/**
 * @brief Free probe results and stop monitoring.
 */
void probe_monitor_free_results(void);

/**
 * @brief Save results to internal flash storage.
 *
 * @return true on success, false on failure.
 */
bool probe_monitor_save_results_to_internal_flash(void);

/**
 * @brief Save results to SD card.
 *
 * @return true on success, false on failure.
 */
bool probe_monitor_save_results_to_sd_card(void);

#ifdef __cplusplus
}
#endif

#endif // PROBE_MONITOR_H
