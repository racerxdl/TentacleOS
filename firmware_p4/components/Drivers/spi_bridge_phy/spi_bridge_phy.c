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

#include "spi_bridge_phy.h"

#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "pin_def.h"
#include "spi.h"

static const char *TAG = "SPI_BRIDGE_PHY";

#define BRIDGE_CLOCK_HZ   (10 * 1000 * 1000)
#define BRIDGE_SPI_MODE   0
#define BRIDGE_QUEUE_SIZE 7

static SemaphoreHandle_t s_irq_semaphore = NULL;

static void IRAM_ATTR irq_handler(void *arg) {
  xSemaphoreGiveFromISR(s_irq_semaphore, NULL);
}

esp_err_t spi_bridge_phy_init(void) {
  s_irq_semaphore = xSemaphoreCreateBinary();

  esp_err_t ret =
      spi_bus_init(SPI2_HOST, GPIO_BRIDGE_MOSI_PIN, GPIO_BRIDGE_MISO_PIN, GPIO_BRIDGE_SCLK_PIN);
  if (ret != ESP_OK) {
    return ret;
  }

  spi_device_config_t devcfg = {
      .cs_pin = GPIO_BRIDGE_CS_PIN,
      .clock_speed_hz = BRIDGE_CLOCK_HZ,
      .mode = BRIDGE_SPI_MODE,
      .queue_size = BRIDGE_QUEUE_SIZE,
  };

  ret = spi_add_device(SPI2_HOST, SPI_DEVICE_BRIDGE, &devcfg);
  if (ret != ESP_OK) {
    return ret;
  }

  gpio_config_t io_conf = {
      .intr_type = GPIO_INTR_POSEDGE,
      .pin_bit_mask = (1ULL << GPIO_BRIDGE_IRQ_PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_down_en = 1,
  };
  gpio_config(&io_conf);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(GPIO_BRIDGE_IRQ_PIN, irq_handler, NULL);

  ESP_LOGI(TAG, "SPI bridge PHY initialized");
  return ESP_OK;
}

esp_err_t spi_bridge_phy_transmit(const uint8_t *tx_data, uint8_t *rx_data, size_t len) {
  spi_device_handle_t handle = spi_get_handle(SPI_DEVICE_BRIDGE);
  if (handle == NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  spi_transaction_t t = {
      .length = len * 8,
      .tx_buffer = tx_data,
      .rx_buffer = rx_data,
  };
  return spi_device_polling_transmit(handle, &t);
}

esp_err_t spi_bridge_phy_wait_irq(uint32_t timeout_ms) {
  if (xSemaphoreTake(s_irq_semaphore, pdMS_TO_TICKS(timeout_ms)) == pdTRUE) {
    return ESP_OK;
  }
  return ESP_ERR_TIMEOUT;
}
