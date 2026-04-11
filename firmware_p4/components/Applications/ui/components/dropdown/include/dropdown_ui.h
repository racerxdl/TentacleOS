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

#ifndef UI_DROPDOWN_H
#define UI_DROPDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/** @brief Create the dropdown panel on the given parent. */
void dropdown_ui_create(lv_obj_t *parent);

/** @brief Register objects to hide when the dropdown opens. */
void dropdown_ui_register_hide_objs(lv_obj_t **objs, int count);

/** @brief Check if the dropdown is currently open. */
bool dropdown_ui_is_open(void);

/** @brief Bring the dropdown to the foreground. */
void dropdown_ui_raise(void);

#ifdef __cplusplus
}
#endif

#endif // UI_DROPDOWN_H
