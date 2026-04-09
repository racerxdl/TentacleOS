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

#ifndef SPI_SLAVE_DRIVER_H
#define SPI_SLAVE_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

/**
 * @brief Initialize the SPI slave driver for P4-to-C5 bridge communication.
 *
 * @return ESP_OK on success, or an error code.
 */
esp_err_t spi_slave_driver_init(void);

/**
 * @brief Wait for and execute an SPI slave transaction.
 *
 * Blocks until the master initiates a transfer, then exchanges data.
 *
 * @param tx_data  Transmit buffer (may be NULL).
 * @param rx_data  Receive buffer (may be NULL).
 * @param len      Number of bytes to transfer.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t spi_slave_driver_transmit(const uint8_t *tx_data, uint8_t *rx_data, size_t len);

/**
 * @brief Set the IRQ output level to signal the master.
 *
 * @param level  GPIO level (0 or 1).
 */
void spi_slave_driver_set_irq(int level);

#ifdef __cplusplus
}
#endif

#endif // SPI_SLAVE_DRIVER_H
