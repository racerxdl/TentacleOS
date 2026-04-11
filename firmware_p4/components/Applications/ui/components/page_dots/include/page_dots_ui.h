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

#ifndef UI_PAGE_DOTS_H
#define UI_PAGE_DOTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#define PAGE_DOTS_MAX 16

typedef struct {
  lv_obj_t *container;
  lv_obj_t *dots[PAGE_DOTS_MAX];
  int total;
  int current;
} page_dots_t;

/** @brief Create a page dots indicator centered on the current page. */
page_dots_t page_dots_create(lv_obj_t *parent, int total, lv_align_t align, int x_ofs, int y_ofs);

/** @brief Update which page is selected. */
void page_dots_set(page_dots_t *pd, int index);

/** @brief Show the page dots. */
void page_dots_show(page_dots_t *pd);

/** @brief Hide the page dots. */
void page_dots_hide(page_dots_t *pd);

#ifdef __cplusplus
}
#endif

#endif // UI_PAGE_DOTS_H
