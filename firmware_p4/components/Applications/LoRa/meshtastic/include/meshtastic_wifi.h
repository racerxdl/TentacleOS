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

#ifndef MESHTASTIC_WIFI_H
#define MESHTASTIC_WIFI_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Meshtastic WiFi AP + TCP server.
 *
 * Creates a WiFi AP and listens on port 4403 for the Meshtastic app.
 * Uses the same protobuf protocol as BLE (StreamAPI framing).
 *
 * @param node_num  This device's node number.
 * @return ESP_OK on success.
 */
esp_err_t meshtastic_wifi_init(uint32_t node_num);

/**
 * @brief Process incoming TCP connections and data. Call from main loop.
 */
void meshtastic_wifi_poll(void);

/**
 * @brief Check if a client is connected.
 */
bool meshtastic_wifi_is_connected(void);

/**
 * @brief Get the current client socket fd. Returns -1 if not connected.
 */
int meshtastic_wifi_get_client_fd(void);

#ifdef __cplusplus
}
#endif

#endif // MESHTASTIC_WIFI_H
