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

#ifndef UI_THEME_H
#define UI_THEME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "lvgl.h"

/** @brief Theme color configuration for the UI. */
typedef struct {
  lv_color_t bg_primary;
  lv_color_t bg_secondary;
  lv_color_t bg_item_top;
  lv_color_t bg_item_bot;
  lv_color_t border_accent;
  lv_color_t border_interface;
  lv_color_t border_inactive;
  lv_color_t text_main;
  lv_color_t screen_base;

  lv_color_t protocol_nfc;
  lv_color_t protocol_wifi;
  lv_color_t protocol_ble;
  lv_color_t protocol_subghz;
  lv_color_t protocol_rfid;
  lv_color_t protocol_ir;
  lv_color_t protocol_lora;
} ui_theme_t;

/** @brief Protocol identifiers for accent color selection. */
typedef enum {
  PROTOCOL_NONE,
  PROTOCOL_NFC,
  PROTOCOL_WIFI,
  PROTOCOL_BLE,
  PROTOCOL_SUBGHZ,
  PROTOCOL_RFID,
  PROTOCOL_IR,
  PROTOCOL_LORA,
  PROTOCOL_COUNT
} protocol_id_t;

extern ui_theme_t current_theme;

/** @brief Initialize the theme system from config or fallback. */
void ui_theme_init(void);

/** @brief Load a built-in theme by index. */
void ui_theme_load_idx(int color_idx);

/** @brief Load a theme by name from SD or flash. */
void ui_theme_load_from_name(const char *theme_name);

/** @brief Load theme from the stored config file. */
void ui_theme_load_from_sd(void);

/** @brief Set the active protocol for accent color. */
void ui_theme_set_protocol(protocol_id_t protocol);

/** @brief Get the accent color for the active protocol. */
lv_color_t ui_theme_get_accent(void);

#ifdef __cplusplus
}
#endif

#endif // UI_THEME_H