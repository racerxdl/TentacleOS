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

#ifndef YS_RFID2_TYPES_H
#define YS_RFID2_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define YS_RFID2_CARD_ID_LEN  10
#define YS_RFID2_RAW_DATA_LEN 5

typedef enum {
  YS_RFID2_STATE_UNINITIALIZED = 0,
  YS_RFID2_STATE_IDLE,
  YS_RFID2_STATE_SCANNING,
  YS_RFID2_STATE_ERROR,
  YS_RFID2_STATE_COUNT
} ys_rfid2_state_t;

typedef enum {
  YS_RFID2_EVENT_CARD_DETECTED = 0,
  YS_RFID2_EVENT_CARD_REMOVED,
  YS_RFID2_EVENT_COUNT
} ys_rfid2_event_type_t;

/**
 * @brief Raw data from the RFID reader module.
 */
typedef struct {
  char id_str[YS_RFID2_CARD_ID_LEN + 1];
  uint8_t data[YS_RFID2_RAW_DATA_LEN];
  uint8_t bit_count;
} ys_rfid2_raw_data_t;

/**
 * @brief Event delivered to the user callback.
 */
typedef struct {
  ys_rfid2_event_type_t type;
  ys_rfid2_raw_data_t raw;
  int64_t timestamp_ms;
} ys_rfid2_event_t;

/**
 * @brief Callback invoked on card events.
 *
 * Called from the reader task context. Must not block for extended periods.
 *
 * @param event  Pointer to event data. Valid only during callback scope.
 * @param ctx    User context pointer provided at registration.
 */
typedef void (*ys_rfid2_event_cb_t)(const ys_rfid2_event_t *event, void *ctx);

/**
 * @brief Driver configuration.
 */
typedef struct {
  int uart_port;
  int baud_rate;
  int tx_pin;
  int rx_pin;
  uint32_t debounce_ms;
  uint32_t removal_timeout_ms;
} ys_rfid2_config_t;

#ifdef __cplusplus
}
#endif

#endif // YS_RFID2_TYPES_H
