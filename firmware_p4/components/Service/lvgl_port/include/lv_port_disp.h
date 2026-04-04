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

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/**
 * @brief Initialize the LVGL display driver.
 *
 * Creates a display instance backed by two DMA-capable draw buffers
 * (partial rendering, 1/5 of screen each). Registers the flush
 * callback that sends pixels to the ST7789 panel via esp_lcd, and
 * optionally mirrors frames to the BLE screen server.
 */
void lv_port_disp_init(void);

#ifdef __cplusplus
}
#endif

#endif // LV_PORT_DISP_H
