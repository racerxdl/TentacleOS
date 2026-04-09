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

#ifndef KEYBOARD_UI_H
#define KEYBOARD_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef void (*keyboard_submit_cb_t)(const char *text, void *user_data);

/** @brief Open the on-screen keyboard. */
void keyboard_open(lv_obj_t *target_textarea, keyboard_submit_cb_t cb, void *user_data);

/** @brief Close the on-screen keyboard. */
void keyboard_close(void);

#ifdef __cplusplus
}
#endif

#endif // KEYBOARD_UI_H
