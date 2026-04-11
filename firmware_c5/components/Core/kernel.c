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

#include "kernel.h"

#include <stdio.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "bq25896.h"
#include "buttons_gpio.h"
#include "console_service.h"
#include "i2c_init.h"
#include "led_control.h"
#include "pin_def.h"
#include "spi.h"
#include "spi_bridge.h"
#include "storage_assets.h"
#include "storage_init.h"
#include "sys_monitor.h"
#include "wifi_service.h"

static const char *TAG = "SAFEGUARD";

#define CONSOLE_TASK_STACK 4096
#define CONSOLE_TASK_PRIO  5
#define BOOT_SETTLE_MS     1500

static void console_task(void *pvParameters) {
  console_service_init();
  vTaskDelete(NULL);
}

void kernel_init(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  spi_init();
  init_i2c();
  // Storage Init
  storage_init();
  storage_assets_init();
  storage_assets_print_info();

  led_rgb_init();
  bq25896_init();
  spi_bridge_slave_init();

  sys_monitor(false);

  wifi_service_init();

  xTaskCreate(console_task, "console_task", CONSOLE_TASK_STACK, NULL, CONSOLE_TASK_PRIO, NULL);

  vTaskDelay(pdMS_TO_TICKS(BOOT_SETTLE_MS));
}

// Safeguards

void safeguard_alert(const char *title, const char *message) {
  ESP_LOGE(TAG, "ALERT: %s - %s", title, message);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  ESP_LOGE(TAG, "!!! CRITICAL STACK OVERFLOW DETECTED !!!");
  ESP_LOGE(TAG, "Task Name: [%s]", pcTaskName);
  ESP_LOGE(TAG, "Task attempted to use more memory than was allocated.");
}

void vApplicationMallocFailedHook(void) {
  ESP_LOGE(TAG, "!!! OUT OF MEMORY (MALLOC FAILED) !!!");
}
