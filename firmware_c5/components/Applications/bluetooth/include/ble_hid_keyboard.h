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

#ifndef BLE_HID_KEYBOARD_H
#define BLE_HID_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

/**
 * @brief Initialize the HID over GATT (HOGP) service and start advertising as a keyboard.
 *
 * @return
 *   - ESP_OK on success
 */
esp_err_t ble_hid_init(void);

/**
 * @brief Stop the HID service and disconnect.
 *
 * @return
 *   - ESP_OK on success
 */
esp_err_t ble_hid_deinit(void);

/**
 * @brief Check if a host is connected and ready to receive keystrokes.
 *
 * @return true if connected, false otherwise.
 */
bool ble_hid_is_connected(void);

/**
 * @brief Send a key press via Bluetooth HID.
 *
 * @param keycode  HID keycode (e.g., 0x04 for 'A').
 * @param modifier Modifier flags (Shift, Ctrl, Alt, GUI).
 */
void ble_hid_send_key(uint8_t keycode, uint8_t modifier);

#ifdef __cplusplus
}
#endif

#endif // BLE_HID_KEYBOARD_H
