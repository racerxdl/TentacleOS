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

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "lvgl.h"

/** Keypad input device (physical buttons mapped to LVGL keys). */
extern lv_indev_t *indev_keypad;

/** Default navigation group. Widgets are auto-added to this group. */
extern lv_group_t *main_group;

/**
 * @brief Initialize the LVGL input device driver.
 *
 * Creates a KEYPAD input device, a default navigation group, and
 * binds them together. Physical buttons are polled via the
 * buttons_gpio driver.
 */
void lv_port_indev_init(void);

/**
 * @brief Toggle keyboard navigation mode.
 *
 * When enabled, UP/DOWN buttons send LV_KEY_UP/LV_KEY_DOWN instead
 * of LV_KEY_PREV/LV_KEY_NEXT. Useful for text input or list scrolling.
 *
 * @param is_enabled  true for keyboard mode, false for navigation mode.
 */
void lv_port_indev_set_keyboard_mode(bool is_enabled);

#ifdef __cplusplus
}
#endif

#endif // LV_PORT_INDEV_H
