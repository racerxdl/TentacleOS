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

#ifndef SD_CARD_INIT_H
#define SD_CARD_INIT_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Configurações de pinos */
#define SD_PIN_MOSI 11
#define SD_PIN_MISO 13
#define SD_PIN_CLK  12
#define SD_PIN_CS   14

/* Configurações gerais */
#define SD_MOUNT_POINT     "/sdcard"
#define SD_MAX_FILES       10
#define SD_ALLOCATION_UNIT (16 * 1024)

/**
 * @brief Inicializa o cartão SD com configuração padrão
 * @return ESP_OK em caso de sucesso
 */
esp_err_t sd_init(void);

/**
 * @brief Inicializa com configuração customizada
 * @param max_files Número máximo de arquivos abertos
 * @param format_if_failed Se deve formatar em caso de falha
 * @return ESP_OK em caso de sucesso
 */
esp_err_t sd_init_custom(uint8_t max_files, bool format_if_failed);

/**
 * @brief Inicializa com pinos customizados
 * @param mosi Pino MOSI
 * @param miso Pino MISO
 * @param clk Pino CLK
 * @param cs Pino CS
 * @return ESP_OK em caso de sucesso
 */
esp_err_t sd_init_custom_pins(int mosi, int miso, int clk, int cs);

/**
 * @brief Desmonta o cartão SD
 * @return ESP_OK em caso de sucesso
 */
esp_err_t sd_deinit(void);

/**
 * @brief Verifica se está montado
 * @return true se montado
 */
bool sd_is_mounted(void);

/**
 * @brief Remonta o cartão
 * @return ESP_OK em caso de sucesso
 */
esp_err_t sd_remount(void);

/**
 * @brief Reseta o barramento SPI
 * @return ESP_OK em caso de sucesso
 */
esp_err_t sd_reset_bus(void);

/**
 * @brief Verifica saúde da conexão
 * @return ESP_OK se saudável
 */
esp_err_t sd_check_health(void);

#ifdef __cplusplus
}
#endif

#endif /* SD_CARD_INIT_H */