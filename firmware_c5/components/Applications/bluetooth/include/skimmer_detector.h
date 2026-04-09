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

#ifndef SKIMMER_DETECTOR_H
#define SKIMMER_DETECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "host/ble_hs.h"

#define SKIMMER_DETECTOR_MAX_RESULTS 50

/**
 * @brief Record for a detected potential BLE skimmer device.
 */
typedef struct {
  ble_addr_t addr;
  int8_t rssi;
  char name[32];
  uint32_t last_seen;
  char detection_reason[32];
} skimmer_detector_record_t;

/**
 * @brief Start scanning for potential BLE skimmer devices.
 *
 * @return true if started, false on failure.
 */
bool skimmer_detector_start(void);

/**
 * @brief Stop the skimmer detection scan.
 */
void skimmer_detector_stop(void);

/**
 * @brief Get the list of detected potential skimmers.
 *
 * @param[out] out_count Pointer to store the number of results.
 * @return Pointer to the results array, or NULL if empty.
 */
skimmer_detector_record_t *skimmer_detector_get_results(uint16_t *out_count);

/**
 * @brief Clear all detected skimmer results.
 */
void skimmer_detector_clear_results(void);

#ifdef __cplusplus
}
#endif

#endif // SKIMMER_DETECTOR_H
