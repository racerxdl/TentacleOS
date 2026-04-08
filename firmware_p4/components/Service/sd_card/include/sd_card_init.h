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
 * @file sd_card_init.h
 * @brief SD card initialization and control (SDMMC 4-bit).
 */

#ifndef SD_CARD_INIT_H
#define SD_CARD_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "vfs_config.h"

#define SD_MAX_FILES       10
#define SD_ALLOCATION_UNIT (16 * 1024)

/**
 * @brief Initialize SD card with default settings (SDMMC 4-bit).
 *
 * @return ESP_OK on success.
 */
esp_err_t sd_init(void);

/**
 * @brief Initialize SD card with custom settings.
 *
 * @param max_files          Maximum open files.
 * @param is_format_on_fail  Format partition if mount fails.
 * @return ESP_OK on success.
 */
esp_err_t sd_init_custom(uint8_t max_files, bool is_format_on_fail);

/**
 * @brief Unmount the SD card and free resources.
 *
 * @return ESP_OK on success.
 */
esp_err_t sd_deinit(void);

/**
 * @brief Check if the SD card is currently mounted.
 *
 * @return true if mounted.
 */
bool sd_is_mounted(void);

/**
 * @brief Remount the SD card (deinit + init).
 *
 * @return ESP_OK on success.
 */
esp_err_t sd_remount(void);

/**
 * @brief Perform a write/read health check.
 *
 * @return ESP_OK if healthy.
 */
esp_err_t sd_check_health(void);

#ifdef __cplusplus
}
#endif

#endif // SD_CARD_INIT_H
