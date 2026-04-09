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

#ifndef SPI_BRIDGE_PHY_H
#define SPI_BRIDGE_PHY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "driver/spi_master.h"

#define BRIDGE_SPI_HOST SPI2_HOST

/**
 * @brief Initialize the SPI bridge physical layer.
 *
 * Configures SPI2 as master for P4-to-C5 communication and sets up
 * the IRQ GPIO interrupt.
 *
 * @return ESP_OK on success, or an error code.
 */
esp_err_t spi_bridge_phy_init(void);

/**
 * @brief Perform a full-duplex SPI transaction on the bridge bus.
 *
 * @param tx_data  Transmit buffer (may be NULL for receive-only).
 * @param rx_data  Receive buffer (may be NULL for transmit-only).
 * @param len      Number of bytes to transfer.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t spi_bridge_phy_transmit(const uint8_t *tx_data, uint8_t *rx_data, size_t len);

/**
 * @brief Wait for an IRQ pulse from the C5 slave.
 *
 * @param timeout_ms  Maximum wait time in milliseconds.
 * @return ESP_OK if IRQ received, ESP_ERR_TIMEOUT otherwise.
 */
esp_err_t spi_bridge_phy_wait_irq(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // SPI_BRIDGE_PHY_H
