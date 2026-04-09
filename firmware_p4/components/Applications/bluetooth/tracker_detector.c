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

#include "tracker_detector.h"

#include <stdlib.h>
#include <string.h>

#include "spi_bridge.h"

#define SPI_TIMEOUT_MS      2000
#define SPI_DATA_TIMEOUT_MS 1000

static tracker_detector_record_t *s_cached_results = NULL;
static uint16_t s_cached_count = 0;
static uint16_t s_cached_capacity = 0;
static tracker_detector_record_t s_empty_record;

bool tracker_detector_start(void) {
  tracker_detector_clear_results();
  return (spi_bridge_send_command(SPI_ID_BT_APP_TRACKER, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS) ==
          ESP_OK);
}

void tracker_detector_stop(void) {
  spi_bridge_send_command(SPI_ID_BT_APP_STOP, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
  tracker_detector_clear_results();
}

tracker_detector_record_t *tracker_detector_get_results(uint16_t *out_count) {
  spi_header_t resp;
  uint8_t payload[2];
  uint16_t magic_count = SPI_DATA_INDEX_COUNT;

  if (spi_bridge_send_command(SPI_ID_SYSTEM_DATA,
                              (uint8_t *)&magic_count,
                              sizeof(magic_count),
                              &resp,
                              payload,
                              SPI_DATA_TIMEOUT_MS) != ESP_OK) {
    if (out_count != NULL) {
      *out_count = s_cached_count;
    }
    return s_cached_results != NULL ? s_cached_results : &s_empty_record;
  }

  uint16_t remote_count = 0;
  memcpy(&remote_count, payload, sizeof(remote_count));
  if (remote_count > TRACKER_DETECTOR_MAX_RESULTS) {
    remote_count = TRACKER_DETECTOR_MAX_RESULTS;
  }

  if (remote_count < s_cached_count) {
    s_cached_count = 0;
  }

  if (remote_count > s_cached_capacity) {
    tracker_detector_record_t *new_buf = (tracker_detector_record_t *)realloc(
        s_cached_results, remote_count * sizeof(tracker_detector_record_t));
    if (new_buf == NULL) {
      if (out_count != NULL) {
        *out_count = s_cached_count;
      }
      return s_cached_results != NULL ? s_cached_results : &s_empty_record;
    }
    s_cached_results = new_buf;
    s_cached_capacity = remote_count;
  }

  uint16_t fetched = s_cached_count;
  for (uint16_t i = s_cached_count; i < remote_count; i++) {
    if (spi_bridge_send_command(SPI_ID_SYSTEM_DATA,
                                (uint8_t *)&i,
                                sizeof(i),
                                &resp,
                                (uint8_t *)&s_cached_results[i],
                                SPI_DATA_TIMEOUT_MS) != ESP_OK) {
      break;
    }
    fetched = i + 1;
  }

  s_cached_count = fetched;
  if (out_count != NULL) {
    *out_count = s_cached_count;
  }
  return s_cached_results != NULL ? s_cached_results : &s_empty_record;
}

void tracker_detector_clear_results(void) {
  if (s_cached_results != NULL) {
    free(s_cached_results);
    s_cached_results = NULL;
  }
  s_cached_count = 0;
  s_cached_capacity = 0;
}
