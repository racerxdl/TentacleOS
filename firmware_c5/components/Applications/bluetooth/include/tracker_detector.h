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

#ifndef TRACKER_DETECTOR_H
#define TRACKER_DETECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "host/ble_hs.h"

#define TRACKER_DETECTOR_MAX_RESULTS 50

/**
 * @brief Known BLE tracker device types.
 */
typedef enum {
  TRACKER_DETECTOR_TYPE_UNKNOWN = 0,
  TRACKER_DETECTOR_TYPE_AIRTAG,
  TRACKER_DETECTOR_TYPE_SMARTTAG,
  TRACKER_DETECTOR_TYPE_TILE,
  TRACKER_DETECTOR_TYPE_CHIPOLO,
  TRACKER_DETECTOR_TYPE_COUNT
} tracker_detector_type_t;

#define TRACKER_DETECTOR_PAYLOAD_SAMPLE_SIZE 10

/**
 * @brief Record for a detected BLE tracker device.
 */
typedef struct {
  ble_addr_t addr;
  int8_t rssi;
  tracker_detector_type_t type;
  char type_str[16];
  uint32_t last_seen;
  uint8_t payload_sample[TRACKER_DETECTOR_PAYLOAD_SAMPLE_SIZE];
} tracker_detector_record_t;

/**
 * @brief Start scanning for BLE tracker devices.
 *
 * @return true if started, false on failure.
 */
bool tracker_detector_start(void);

/**
 * @brief Stop the tracker detection scan.
 */
void tracker_detector_stop(void);

/**
 * @brief Get the list of detected tracker devices.
 *
 * @param[out] out_count Pointer to store the number of results.
 * @return Pointer to the results array, or NULL if empty.
 */
tracker_detector_record_t *tracker_detector_get_results(uint16_t *out_count);

/**
 * @brief Clear all detected tracker results.
 */
void tracker_detector_clear_results(void);

#ifdef __cplusplus
}
#endif

#endif // TRACKER_DETECTOR_H
