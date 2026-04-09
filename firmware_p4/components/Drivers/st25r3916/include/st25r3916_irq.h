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

#ifndef ST25R3916_IRQ_H
#define ST25R3916_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

/**
 * @brief Snapshot of all five IRQ status registers.
 */
typedef struct {
  uint8_t main;      /**< REG_MAIN_INT (0x1A) */
  uint8_t timer;     /**< REG_TIMER_NFC_INT (0x1B) */
  uint8_t error;     /**< REG_ERROR_INT (0x1C) */
  uint8_t target;    /**< REG_TARGET_INT (0x1D) */
  uint8_t collision; /**< REG_COLLISION (0x20) */
} st25r3916_irq_status_t;

/**
 * @brief Read and clear all five IRQ status registers.
 *
 * @param[out] out_status Pointer to store the result.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if pointer is NULL.
 */
esp_err_t st25r3916_irq_read(st25r3916_irq_status_t *out_status);

/**
 * @brief Read IRQ status and log all fields at WARN level.
 *
 * @param ctx Context string.
 * @param fifo_count Current FIFO byte count.
 */
void st25r3916_irq_log(const char *ctx, uint16_t fifo_count);

/**
 * @brief Poll the IRQ pin until TXE is asserted or timeout expires.
 *
 * @return
 * - ESP_OK if TXE was received.
 * - ESP_ERR_TIMEOUT on timeout.
 * - ESP_FAIL on hardware error.
 */
esp_err_t st25r3916_irq_wait_txe(void);

#ifdef __cplusplus
}
#endif

#endif // ST25R3916_IRQ_H
