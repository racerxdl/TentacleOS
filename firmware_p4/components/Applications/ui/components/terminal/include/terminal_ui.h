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

#ifndef UI_TERMINAL_H
#define UI_TERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/** @brief Create a styled terminal text area. */
lv_obj_t *terminal_ui_create(lv_obj_t *parent,
                             lv_coord_t width,
                             lv_coord_t height,
                             lv_align_t align,
                             lv_coord_t x_ofs,
                             lv_coord_t y_ofs);

#ifdef __cplusplus
}
#endif

#endif // UI_TERMINAL_H
