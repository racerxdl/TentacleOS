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

#ifndef TUSB_DESC_H
#define TUSB_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "tinyusb.h"

#define TUSB_DESC_ITF_NUM_HID 0

/**
 * @brief Initialize the TinyUSB driver with HID composite descriptors.
 *
 * Installs the GPIO ISR service (required for ESP32-P4 High Speed USB),
 * configures device/configuration/HID report descriptors, and installs
 * the TinyUSB driver.
 *
 * Must be called before any HID report transmission.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL if the TinyUSB driver installation fails
 */
esp_err_t busb_init(void);

#ifdef __cplusplus
}
#endif

#endif // TUSB_DESC_H
