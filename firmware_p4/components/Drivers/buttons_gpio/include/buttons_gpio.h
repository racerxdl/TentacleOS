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

#ifndef BUTTONS_GPIO_H
#define BUTTONS_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "pin_def.h"

typedef struct {
  uint32_t gpio;
  bool last_state;
  bool pressed_flag;
} button_t;

/**
 * @brief Initialize GPIO buttons.
 */
void buttons_init(void);

/**
 * @brief Poll button states. Call periodically from a task.
 */
void buttons_task(void);

/**
 * @brief Check if the up button was pressed since last check.
 *
 * @return true if pressed, false otherwise.
 */
bool up_button_pressed(void);

/**
 * @brief Check if the down button was pressed since last check.
 *
 * @return true if pressed, false otherwise.
 */
bool down_button_pressed(void);

/**
 * @brief Check if the left button was pressed since last check.
 *
 * @return true if pressed, false otherwise.
 */
bool left_button_pressed(void);

/**
 * @brief Check if the right button was pressed since last check.
 *
 * @return true if pressed, false otherwise.
 */
bool right_button_pressed(void);

/**
 * @brief Check if the ok button was pressed since last check.
 *
 * @return true if pressed, false otherwise.
 */
bool ok_button_pressed(void);

/**
 * @brief Check if the back button was pressed since last check.
 *
 * @return true if pressed, false otherwise.
 */
bool back_button_pressed(void);

/**
 * @brief Check if the up button is currently held down.
 *
 * @return true if held, false otherwise.
 */
bool up_button_is_down(void);

/**
 * @brief Check if the down button is currently held down.
 *
 * @return true if held, false otherwise.
 */
bool down_button_is_down(void);

/**
 * @brief Check if the left button is currently held down.
 *
 * @return true if held, false otherwise.
 */
bool left_button_is_down(void);

/**
 * @brief Check if the right button is currently held down.
 *
 * @return true if held, false otherwise.
 */
bool right_button_is_down(void);

/**
 * @brief Check if the ok button is currently held down.
 *
 * @return true if held, false otherwise.
 */
bool ok_button_is_down(void);

/**
 * @brief Check if the back button is currently held down.
 *
 * @return true if held, false otherwise.
 */
bool back_button_is_down(void);

#ifdef __cplusplus
}
#endif

#endif // BUTTONS_GPIO_H
