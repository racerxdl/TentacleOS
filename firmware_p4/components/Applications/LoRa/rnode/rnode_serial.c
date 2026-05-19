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

#include "rnode_serial.h"

#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "RNODE_SERIAL";

#define RNODE_UART_PORT   UART_NUM_0
#define RNODE_UART_TX_PIN 43
#define RNODE_UART_RX_PIN 44
#define RNODE_UART_BAUD   115200
#define RNODE_UART_RX_BUF 2048
#define RNODE_UART_TX_BUF 2048

esp_err_t rnode_serial_init(void) {
  uart_config_t cfg = {
      .baud_rate = RNODE_UART_BAUD,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  esp_err_t ret =
      uart_driver_install(RNODE_UART_PORT, RNODE_UART_RX_BUF, RNODE_UART_TX_BUF, 0, NULL, 0);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "uart_driver_install failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = uart_param_config(RNODE_UART_PORT, &cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "uart_param_config failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = uart_set_pin(RNODE_UART_PORT,
                     RNODE_UART_TX_PIN,
                     RNODE_UART_RX_PIN,
                     UART_PIN_NO_CHANGE,
                     UART_PIN_NO_CHANGE);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "uart_set_pin failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG,
           "UART%d @ %d baud (TX=GPIO%d RX=GPIO%d)",
           RNODE_UART_PORT,
           RNODE_UART_BAUD,
           RNODE_UART_TX_PIN,
           RNODE_UART_RX_PIN);
  return ESP_OK;
}

int rnode_serial_write(const uint8_t *data, size_t len) {
  if (data == NULL || len == 0) {
    return 0;
  }
  int n = uart_write_bytes(RNODE_UART_PORT, (const char *)data, len);
  return n;
}

int rnode_serial_read(uint8_t *out, size_t cap) {
  if (out == NULL || cap == 0) {
    return 0;
  }
  int n = uart_read_bytes(RNODE_UART_PORT, out, cap, 0);
  if (n < 0) {
    return 0;
  }
  return n;
}
