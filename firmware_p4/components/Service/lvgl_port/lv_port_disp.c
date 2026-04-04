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

#include "lv_port_disp.h"

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"

#include "st7789.h"
#include "ble_screen_server.h"

static const char *TAG = "LV_PORT_DISP";

#define LVGL_BUF_LINES  (LCD_V_RES / 5)
#define LVGL_BUF_PIXELS (LCD_H_RES * LVGL_BUF_LINES)
#define LVGL_BUF_ALLOC  (MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)

static lv_display_t *s_disp_handle = NULL;

static bool flush_ready_cb(esp_lcd_panel_io_handle_t panel_io,
                           esp_lcd_panel_io_event_data_t *edata,
                           void *user_ctx) {
  lv_display_t *disp = (lv_display_t *)user_ctx;
  lv_display_flush_ready(disp);
  return false;
}

static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  uint32_t px_count = w * h;

  lv_draw_sw_rgb565_swap(px_map, px_count);

  esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);

  if (ble_screen_server_is_active()) {
    ble_screen_server_send_partial((const uint16_t *)px_map, area->x1, area->y1, w, h);
  }
}

void lv_port_disp_init(void) {
  s_disp_handle = lv_display_create(LCD_H_RES, LCD_V_RES);
  lv_display_set_flush_cb(s_disp_handle, disp_flush);

  size_t buf_size = LVGL_BUF_PIXELS * sizeof(lv_color_t);

  void *buf1 = heap_caps_malloc(buf_size, LVGL_BUF_ALLOC);
  void *buf2 = heap_caps_malloc(buf_size, LVGL_BUF_ALLOC);

  if (buf1 == NULL || buf2 == NULL) {
    ESP_LOGE(TAG, "Failed to allocate display buffers (%u bytes each)", (unsigned)buf_size);
    return;
  }

  lv_display_set_buffers(s_disp_handle, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

  const esp_lcd_panel_io_callbacks_t cbs = {
      .on_color_trans_done = flush_ready_cb,
  };
  esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, s_disp_handle);

  ESP_LOGI(TAG,
           "Display port initialized (%dx%d, buf: %u bytes x2)",
           LCD_H_RES,
           LCD_V_RES,
           (unsigned)buf_size);
}
