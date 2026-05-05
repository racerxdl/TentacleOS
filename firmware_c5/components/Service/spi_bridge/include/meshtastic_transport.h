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

#ifndef MESHTASTIC_TRANSPORT_H
#define MESHTASTIC_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

#include "spi_protocol.h"

/**
 * @brief Initialize the Meshtastic transport core.
 *
 * Allocates internal mutexes and clears reassembly state. Idempotent.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_NO_MEM if mutex allocation fails
 */
esp_err_t meshtastic_transport_init(void);

/**
 * @brief Inject a FromRadio chunk received from the P4 over SPI.
 *
 * Reassembles by seq. When the last chunk arrives, increments the
 * fromnum counter and fans out the assembled frame to BLE GATT
 * (FROMRADIO + FROMNUM notify) and TCP (framed StreamAPI write).
 *
 * @param payload  Raw SPI command payload (chunk header + data).
 * @param len      Length of payload in bytes.
 */
void meshtastic_transport_inject_fromradio_chunk(const uint8_t *payload, uint8_t len);

/**
 * @brief Inject a Logradio chunk received from the P4 over SPI.
 *
 * Reassembles by seq. When the last chunk arrives, dispatches the
 * assembled bytes to BLE GATT LOGRADIO notify if subscribed.
 *
 * @param payload  Raw SPI command payload (chunk header + data).
 * @param len      Length of payload in bytes.
 */
void meshtastic_transport_inject_log_chunk(const uint8_t *payload, uint8_t len);

/**
 * @brief Forward a ToRadio frame received from a phone to the P4.
 *
 * Splits the frame into chunks of up to SPI_MESH_CHUNK_PAYLOAD_MAX bytes,
 * tags each with a per-pipe seq, and pushes them via spi_bridge stream
 * SPI_ID_MESH_TORADIO_STREAM for the P4 to reassemble and feed phoneapi.
 *
 * @param frame  ToRadio protobuf bytes (no transport framing).
 * @param len    Length of frame in bytes.
 * @return true if all chunks were enqueued, false if the stream queue
 *         filled up.
 */
bool meshtastic_transport_send_toradio(const uint8_t *frame, uint16_t len);

/**
 * @brief Fill a status snapshot consumed by SPI_ID_MESH_STATUS responses.
 *
 * Aggregates connection state from the BLE GATT and TCP transports plus
 * the current fromnum counter.
 *
 * @param[out] out_status  Destination struct. Must not be NULL.
 */
void meshtastic_transport_get_status(spi_mesh_status_t *out_status);

/**
 * @brief Get and bump the fromnum counter.
 *
 * Internal helper used by GATT to seed FROMNUM notifications.
 *
 * @return The new counter value (post-increment).
 */
uint32_t meshtastic_transport_bump_fromnum(void);

/**
 * @brief Reset reassembly state and the fromnum counter.
 *
 * Called when both transports go offline so the next session starts
 * with a clean handshake.
 */
void meshtastic_transport_reset(void);

#ifdef __cplusplus
}
#endif

#endif // MESHTASTIC_TRANSPORT_H
