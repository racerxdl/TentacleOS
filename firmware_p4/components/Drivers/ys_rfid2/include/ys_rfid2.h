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

#ifndef YS_RFID2_H
#define YS_RFID2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

#include "ys_rfid2_types.h"

/**
 * @brief Return a default configuration.
 *
 * Defaults: UART_NUM_2, 9600 baud, pins from pin_def.h,
 * 1000 ms debounce, 2000 ms removal timeout.
 */
ys_rfid2_config_t ys_rfid2_default_config(void);

/**
 * @brief Initialize the YS-RFID2 driver.
 *
 * Sets up the UART peripheral via HAL. Does NOT start scanning.
 *
 * @param config  Pointer to configuration. If NULL, uses default config.
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_STATE if already initialized
 *   - ESP_ERR_NO_MEM if mutex creation fails
 */
esp_err_t ys_rfid2_init(const ys_rfid2_config_t *config);

/**
 * @brief Deinitialize the driver and release all resources.
 *
 * Stops scanning if active.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_STATE if not initialized
 */
esp_err_t ys_rfid2_deinit(void);

/**
 * @brief Start continuous card scanning.
 *
 * Creates a background FreeRTOS task that reads the UART and delivers
 * events via the registered callback.
 *
 * @param cb   Event callback function. Must not be NULL.
 * @param ctx  User context pointer passed to the callback.
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_ARG if cb is NULL
 *   - ESP_ERR_INVALID_STATE if not initialized or already scanning
 *   - ESP_ERR_NO_MEM if task creation fails
 */
esp_err_t ys_rfid2_start(ys_rfid2_event_cb_t cb, void *ctx);

/**
 * @brief Stop scanning and destroy the background task.
 *
 * Blocks until the task has exited.
 */
void ys_rfid2_stop(void);

/**
 * @brief Get the current driver state.
 */
ys_rfid2_state_t ys_rfid2_get_state(void);

/**
 * @brief Get the last detected card event.
 *
 * @param[out] out_event  Pointer to receive the event data.
 * @return
 *   - ESP_OK if a card was previously detected
 *   - ESP_ERR_NOT_FOUND if no card has been detected yet
 *   - ESP_ERR_INVALID_ARG if out_event is NULL
 */
esp_err_t ys_rfid2_get_last_card(ys_rfid2_event_t *out_event);

#ifdef __cplusplus
}
#endif

#endif // YS_RFID2_H
