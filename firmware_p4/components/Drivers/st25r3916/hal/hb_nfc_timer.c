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

/**
 * @file hb_nfc_timer.c
 * @brief HAL timer delay utilities: busy-wait (µs) and scheduler-yield (ms).
 */
#include "hb_nfc_timer.h"

#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "hb_nfc_timer";

void hb_nfc_timer_delay_us(uint32_t us) {
  esp_rom_delay_us(us);
}

void hb_nfc_timer_delay_ms(uint32_t ms) {
  /* Yield to the scheduler rather than busy-spinning.
   * Minimum 1 ms to guarantee at least one yield even for ms == 0. */
  ESP_LOGD(TAG, "delay %lu ms", (unsigned long)ms);
  vTaskDelay(pdMS_TO_TICKS(ms != 0 ? ms : 1));
}
