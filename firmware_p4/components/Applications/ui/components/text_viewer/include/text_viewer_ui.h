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

#ifndef UI_TEXT_VIEWER_H
#define UI_TEXT_VIEWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct {
  lv_obj_t *screen;
  lv_obj_t *title_label;
  lv_obj_t *line_label;
  lv_obj_t *text_area;
  lv_obj_t *scroll_bar;
  int track_y_start;
  int track_h;
} text_viewer_t;

/** @brief Create a full-screen text viewer with title and scrollable content. */
text_viewer_t text_viewer_create(lv_obj_t *parent, const char *filename);

/** @brief Load text content from a file path. */
void text_viewer_load_file(text_viewer_t *tv, const char *path);

/** @brief Set text content directly. */
void text_viewer_set_text(text_viewer_t *tv, const char *text);

#ifdef __cplusplus
}
#endif

#endif // UI_TEXT_VIEWER_H
