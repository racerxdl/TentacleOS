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

#ifndef ST25R3916_FIFO_H
#define ST25R3916_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "esp_err.h"

/**
 * @brief Get the number of bytes currently in the FIFO.
 *
 * @return Byte count (0–512).
 */
uint16_t st25r3916_fifo_get_count(void);

/**
 * @brief Clear the FIFO via ST25R3916_CMD_CLEAR direct command.
 */
void st25r3916_fifo_clear(void);

/**
 * @brief Load data into the FIFO for transmission.
 *
 * @param data  Source buffer. Must not be NULL.
 * @param len   Number of bytes to load.
 * @return
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG if data is NULL
 */
esp_err_t st25r3916_fifo_load(const uint8_t *data, size_t len);

/**
 * @brief Read data from the FIFO after reception.
 *
 * @param[out] out_data  Destination buffer. Must not be NULL.
 * @param len            Number of bytes to read.
 * @return
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG if out_data is NULL
 */
esp_err_t st25r3916_fifo_read(uint8_t *out_data, size_t len);

/**
 * @brief Set the TX byte and partial-bit count registers.
 *
 * @param nbytes    Total number of complete bytes to transmit.
 * @param nbtx_bits Extra bits in the final byte.
 */
void st25r3916_fifo_set_tx_bytes(uint16_t nbytes, uint8_t nbtx_bits);

/**
 * @brief Poll the FIFO until it contains at least min_bytes.
 *
 * @param min_bytes         Target byte count.
 * @param timeout_ms        Maximum wait time.
 * @param[out] out_final_count Final FIFO count. May be NULL.
 * @return
 * - ESP_OK if condition met
 * - ESP_ERR_TIMEOUT if target not reached
 */
esp_err_t st25r3916_fifo_wait(size_t min_bytes, int32_t timeout_ms, uint16_t *out_final_count);

#ifdef __cplusplus
}
#endif

#endif // ST25R3916_FIFO_H
