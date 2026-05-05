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

#ifndef MESHTASTIC_GATT_H
#define MESHTASTIC_GATT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

/**
 * @brief Bring up the Meshtastic GATT peripheral on the C5.
 *
 * Initializes NimBLE (idempotent) with Secure Connections, MITM, and
 * display-only IO capability so a fresh 6-digit passkey is generated
 * per pairing. Registers the Meshtastic service UUID
 * 6ba1b218-15a8-461f-9fa8-5dcae273eafd with TORADIO/FROMRADIO/FROMNUM/
 * LOGRADIO characteristics plus the standard Battery Service. Starts
 * advertising as Highboy_<id16>.
 *
 * @param node_num  Our node number.
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_STATE if already running
 *   - Error code from NimBLE otherwise
 */
esp_err_t meshtastic_gatt_init(uint32_t node_num);

/**
 * @brief Stop advertising and tear down the Meshtastic GATT.
 */
void meshtastic_gatt_stop(void);

/**
 * @brief Whether NimBLE is up and advertising the Meshtastic service.
 */
bool meshtastic_gatt_is_running(void);

/**
 * @brief Whether a phone is currently connected over BLE.
 */
bool meshtastic_gatt_is_connected(void);

/**
 * @brief Whether the peer is subscribed to FROMNUM notifications.
 */
bool meshtastic_gatt_is_fromnum_subscribed(void);

/**
 * @brief Whether the peer is subscribed to LOGRADIO notifications.
 */
bool meshtastic_gatt_is_logradio_subscribed(void);

/**
 * @brief Enqueue a fully assembled FromRadio frame for the BLE peer.
 *
 * The next FROMRADIO read returns one of these queued frames. Also
 * triggers a FROMNUM notification when the peer is subscribed.
 *
 * @param frame  FromRadio protobuf bytes (no framing).
 * @param len    Length of frame in bytes.
 */
void meshtastic_gatt_enqueue_fromradio(const uint8_t *frame, uint16_t len);

/**
 * @brief Notify the peer with a Logradio chunk.
 *
 * No-op if no subscriber is present.
 *
 * @param chunk  Log bytes.
 * @param len    Length of chunk in bytes.
 */
void meshtastic_gatt_notify_log(const uint8_t *chunk, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // MESHTASTIC_GATT_H
