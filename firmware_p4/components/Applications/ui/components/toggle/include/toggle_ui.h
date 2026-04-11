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

#ifndef UI_TOGGLE_H
#define UI_TOGGLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "lvgl.h"

typedef struct {
  lv_obj_t *obj;
  lv_obj_t *dot;
  bool state;
} toggle_ui_t;

/** @brief Create a toggle switch on the given parent. */
void toggle_ui_create(toggle_ui_t *toggle, lv_obj_t *parent);

/** @brief Set the toggle state. */
void toggle_ui_set(toggle_ui_t *toggle, bool on);

/** @brief Toggle the current state. */
void toggle_ui_toggle(toggle_ui_t *toggle);

/** @brief Get the current toggle state. */
bool toggle_ui_get(toggle_ui_t *toggle);

#ifdef __cplusplus
}
#endif

#endif // UI_TOGGLE_H
