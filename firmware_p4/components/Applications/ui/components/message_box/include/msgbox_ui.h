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

#ifndef MSGBOX_UI_H
#define MSGBOX_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef void (*msgbox_cb_t)(bool confirm);

/** @brief Open a message box with optional OK and Cancel buttons. */
void msgbox_open(
    const char *icon, const char *msg, const char *btn_ok, const char *btn_cancel, msgbox_cb_t cb);

/** @brief Close the currently open message box. */
void msgbox_close(void);

/** @brief Check if a message box is currently open. */
bool msgbox_is_open(void);

#ifdef __cplusplus
}
#endif

#endif // MSGBOX_UI_H