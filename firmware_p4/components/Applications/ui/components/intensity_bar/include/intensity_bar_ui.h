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

#ifndef UI_INTENSITY_BAR_H
#define UI_INTENSITY_BAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#define INTENSITY_BAR_STEPS 5

typedef struct {
  lv_obj_t *obj;
  lv_obj_t *bars[INTENSITY_BAR_STEPS];
  int level;
} intensity_bar_t;

/** @brief Create an intensity bar on the given parent. */
void intensity_bar_create(intensity_bar_t *ib, lv_obj_t *parent);

/** @brief Set the intensity bar level. */
void intensity_bar_set(intensity_bar_t *ib, int level);

/** @brief Get the current intensity bar level. */
int intensity_bar_get(intensity_bar_t *ib);

/** @brief Increment the intensity bar level by one step. */
void intensity_bar_inc(intensity_bar_t *ib);

/** @brief Decrement the intensity bar level by one step. */
void intensity_bar_dec(intensity_bar_t *ib);

#ifdef __cplusplus
}
#endif

#endif // UI_INTENSITY_BAR_H
