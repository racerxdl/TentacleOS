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

#ifndef HIGHBOY_NFC_H
#define HIGHBOY_NFC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

/**
 * @brief Hardware and bus configuration for library initialization.
 *
 * Target: ST25R3916 / ST25R3916B.
 */
typedef struct {
  gpio_num_t pin_mosi;   /**< SPI MOSI pin. */
  gpio_num_t pin_miso;   /**< SPI MISO pin. */
  gpio_num_t pin_sclk;   /**< SPI clock pin. */
  gpio_num_t pin_cs;     /**< SPI chip-select pin. */
  gpio_num_t pin_irq;    /**< ST25R3916 IRQ output pin. */
  int spi_host;          /**< ESP-IDF SPI host index (e.g., SPI2_HOST). */
  uint8_t spi_mode;      /**< SPI mode (ST25R3916 requires Mode 1). */
  uint32_t spi_clock_hz; /**< SPI clock frequency (Max 6MHz). */
} highboy_nfc_config_t;

/**
 * @brief Default configuration based on ESP32-P4 reference wiring.
 */
#define HIGHBOY_NFC_CONFIG_DEFAULT() \
  {                                  \
      .pin_mosi = GPIO_NUM_18,       \
      .pin_miso = GPIO_NUM_19,       \
      .pin_sclk = GPIO_NUM_17,       \
      .pin_cs = GPIO_NUM_3,          \
      .pin_irq = GPIO_NUM_8,         \
      .spi_host = SPI2_HOST,         \
      .spi_mode = 1,                 \
      .spi_clock_hz = 500000,        \
  }

/**
 * @brief Initialize the NFC library and underlying hardware.
 *
 * @param[in] config Hardware configuration. Must not be NULL.
 * @return
 * - ESP_OK: Success.
 * - ESP_ERR_INVALID_ARG: NULL configuration.
 * - ESP_ERR_NOT_FOUND: Chip not detected.
 * - ESP_FAIL: Hardware initialization failed.
 */
esp_err_t highboy_nfc_init(const highboy_nfc_config_t *config);

/**
 * @brief Deinitialize the library and power down the RF field.
 */
void highboy_nfc_deinit(void);

/**
 * @brief Ping the chip to verify communication.
 *
 * @param[out] out_chip_id Optional pointer to store the raw IC identity.
 * @return ESP_OK on success.
 */
esp_err_t highboy_nfc_ping(uint8_t *out_chip_id);

/**
 * @brief Enable the RF field.
 * @return ESP_OK on success.
 */
esp_err_t highboy_nfc_field_on(void);

/**
 * @brief Disable the RF field.
 * @return ESP_OK on success.
 */
esp_err_t highboy_nfc_field_off(void);

/**
 * @brief Measure the current RF field amplitude.
 *
 * @return Raw ADC value (0–255).
 */
uint8_t highboy_nfc_measure_amplitude(void);

/**
 * @brief Detect if an external RF field is present (External Field Detector).
 *
 * @param[out] out_aux_display Optional pointer to store raw AUX_DISPLAY register.
 * @return true if field detected, false otherwise.
 */
bool highboy_nfc_field_detected(uint8_t *out_aux_display);

#ifdef __cplusplus
}
#endif

#endif // HIGHBOY_NFC_H
