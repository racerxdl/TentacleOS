// Copyright (c) 2025 HIGH CODE LLC
//
// TentacleOS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TentacleOS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TentacleOS. If not, see <https://www.gnu.org/licenses/>.

#include "lv_port_indev.h"

#include "esp_log.h"
#include "core/lv_group.h"

#include "buttons_gpio.h"

static const char *TAG = "LV_PORT_INDEV";

lv_indev_t *indev_keypad = NULL;
lv_group_t *main_group = NULL;

static bool s_is_keyboard_mode = false;

static void keypad_read(lv_indev_t *indev, lv_indev_data_t *data);
static uint32_t keypad_get_key(void);

void lv_port_indev_init(void) {
  indev_keypad = lv_indev_create();
  lv_indev_set_type(indev_keypad, LV_INDEV_TYPE_KEYPAD);
  lv_indev_set_read_cb(indev_keypad, keypad_read);

  main_group = lv_group_create();
  lv_group_set_default(main_group);
  lv_indev_set_group(indev_keypad, main_group);

  ESP_LOGI(TAG, "Input device initialized (keypad + navigation group)");
}

void lv_port_indev_set_keyboard_mode(bool is_enabled) {
  s_is_keyboard_mode = is_enabled;
}

static void keypad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  static uint32_t s_last_key = 0;

  uint32_t key = keypad_get_key();

  if (key != 0) {
    data->state = LV_INDEV_STATE_PRESSED;
    s_last_key = key;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  data->key = s_last_key;
}

static uint32_t keypad_get_key(void) {
  if (up_button_is_down()) {
    return s_is_keyboard_mode ? LV_KEY_UP : LV_KEY_PREV;
  }
  if (down_button_is_down()) {
    return s_is_keyboard_mode ? LV_KEY_DOWN : LV_KEY_NEXT;
  }
  if (ok_button_is_down()) {
    return LV_KEY_ENTER;
  }
  if (back_button_is_down()) {
    return LV_KEY_ESC;
  }
  if (left_button_is_down()) {
    return LV_KEY_LEFT;
  }
  if (right_button_is_down()) {
    return LV_KEY_RIGHT;
  }

  return 0;
}
