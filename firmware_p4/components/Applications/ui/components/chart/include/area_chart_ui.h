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

#ifndef UI_AREA_CHART_H
#define UI_AREA_CHART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#define AREA_CHART_MAX_POINTS 40

typedef void (*area_chart_btn_cb_t)(int btn_index);

typedef struct {
  lv_obj_t *screen;
  lv_obj_t *chart;
  lv_chart_series_t *series;
  lv_obj_t *panel;
  lv_obj_t *btns[2];
  int btn_count;
  int btn_sel;
  area_chart_btn_cb_t btn_cb;
} area_chart_ui_t;

/** @brief Create an area chart screen with bottom button panel. */
area_chart_ui_t area_chart_ui_create(lv_obj_t *parent);

/** @brief Add a button to the bottom panel (max 2). */
void area_chart_ui_add_btn(area_chart_ui_t *ch, const char *label, lv_color_t dot_color);

/** @brief Set the button press callback. */
void area_chart_ui_set_btn_cb(area_chart_ui_t *ch, area_chart_btn_cb_t cb);

/** @brief Dim or undim a button by index. */
void area_chart_ui_set_btn_dim(area_chart_ui_t *ch, int index, bool dim);

/** @brief Append a data point to the chart. */
void area_chart_ui_add_point(area_chart_ui_t *ch, int32_t value);

/** @brief Set all chart points at once. */
void area_chart_ui_set_points(area_chart_ui_t *ch, const int32_t *values, int count);

/** @brief Move button selection to the next button. */
void area_chart_ui_btn_next(area_chart_ui_t *ch);

/** @brief Move button selection to the previous button. */
void area_chart_ui_btn_prev(area_chart_ui_t *ch);

/** @brief Trigger a press on the currently selected button. */
void area_chart_ui_btn_press(area_chart_ui_t *ch);

#ifdef __cplusplus
}
#endif

#endif // UI_AREA_CHART_H
