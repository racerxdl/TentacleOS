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

#ifndef BLE_SCREEN_SERVER_H
#define BLE_SCREEN_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

/**
 * @brief Initialize the BLE screen streaming GATT service.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_NO_MEM if allocation fails
 */
esp_err_t ble_screen_server_init(void);

/**
 * @brief Deinitialize the BLE screen streaming service and free resources.
 */
void ble_screen_server_deinit(void);

/**
 * @brief Send a partial screen update over BLE notifications.
 *
 * @param px_map Pointer to pixel data (RGB565). Must not be NULL.
 * @param x      X offset of the partial region.
 * @param y      Y offset of the partial region.
 * @param w      Width of the partial region.
 * @param h      Height of the partial region.
 */
void ble_screen_server_send_partial(const uint16_t *px_map, int x, int y, int w, int h);

/**
 * @brief Check if the screen server is initialized and active.
 *
 * @return true if active, false otherwise.
 */
bool ble_screen_server_is_active(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_SCREEN_SERVER_H
