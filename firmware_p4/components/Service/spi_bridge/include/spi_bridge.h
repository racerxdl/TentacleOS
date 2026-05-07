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

#ifndef SPI_BRIDGE_H
#define SPI_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#include "spi_protocol.h"

/**
 * @brief Callback invoked when a stream frame is received from the slave.
 *
 * @param id      SPI function identifier of the stream.
 * @param payload Pointer to the frame payload data.
 * @param len     Length of the payload in bytes.
 */
typedef void (*spi_stream_cb_t)(spi_id_t id, const uint8_t *payload, uint8_t len);

/**
 * @brief Initialize the SPI master bridge.
 *
 * @return
 *   - ESP_OK on success
 *   - Error code from the SPI PHY driver on failure
 */
esp_err_t spi_bridge_master_init(void);

/**
 * @brief Return the timeout (ms) for a given SPI command ID.
 *
 * Returns SPI_TIMEOUT_DEFAULT_MS unless the command is in the override
 * table (e.g. WiFi commands which need more time).
 *
 * @param id  SPI command identifier.
 * @return    Timeout in milliseconds.
 */
uint32_t spi_bridge_get_timeout(spi_id_t id);

/**
 * @brief Mark the bridge as alive or dead.
 *
 * When dead, spi_bridge_send_command returns ESP_ERR_INVALID_STATE
 * immediately without attempting transmission. Used to silence the
 * polling spam when the C5 SPI bridge is not connected/responding.
 *
 * @param alive true if the C5 is responding, false otherwise.
 */
void spi_bridge_set_alive(bool alive);

/**
 * @brief Send a command to the SPI slave and receive the response.
 *
 * @param id          Command identifier.
 * @param payload     Pointer to the command payload (may be NULL).
 * @param len         Payload length in bytes.
 * @param out_header  Pointer to store the response header (may be NULL).
 * @param out_payload Buffer to store the response payload (may be NULL).
 * @param timeout_ms  Maximum time to wait for the response in milliseconds.
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_TIMEOUT if the slave did not respond in time
 *   - ESP_ERR_INVALID_ARG if payload length exceeds the maximum
 *   - ESP_ERR_INVALID_RESPONSE on protocol errors
 */
esp_err_t spi_bridge_send_command(spi_id_t id,
                                  const uint8_t *payload,
                                  uint8_t len,
                                  spi_header_t *out_header,
                                  uint8_t *out_payload,
                                  uint32_t timeout_ms);

/**
 * @brief Register a callback for stream frames with the given SPI ID.
 *
 * @param id  SPI function identifier to listen for.
 * @param cb  Callback function to invoke on stream frames.
 */
void spi_bridge_register_stream_cb(spi_id_t id, spi_stream_cb_t cb);

/**
 * @brief Unregister a previously registered stream callback.
 *
 * @param id  SPI function identifier to stop listening for.
 */
void spi_bridge_unregister_stream_cb(spi_id_t id);

#ifdef __cplusplus
}
#endif

#endif // SPI_BRIDGE_H
