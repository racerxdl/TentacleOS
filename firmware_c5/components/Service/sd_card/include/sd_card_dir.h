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

#ifndef SD_CARD_DIR_H
#define SD_CARD_DIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#define SD_BASE_PATH "/sdcard"

#define SD_DIR_IR     "/ir"
#define SD_DIR_BADUSB "/badusb"
#define SD_DIR_NFC    "/nfc"
#define SD_DIR_RFID   "/rfid"
#define SD_DIR_SUBGHZ "/subghz"
#define SD_DIR_CONFIG "/config"
#define SD_DIR_LOGS   "/logs"
#define SD_DIR_BACKUP "/backups"

typedef void (*sd_dir_callback_t)(const char *name, bool is_dir, void *user_data);

/**
 * @brief Create a directory on the SD card.
 *
 * @param path Relative path under the SD mount point.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t sd_dir_create(const char *path);

/**
 * @brief Recursively remove a directory and all its contents.
 *
 * @param path Relative path under the SD mount point.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t sd_dir_remove_recursive(const char *path);

/**
 * @brief Check if a directory exists.
 *
 * @param path Relative path under the SD mount point.
 * @return true if the directory exists.
 */
bool sd_dir_exists(const char *path);

/**
 * @brief List directory contents via callback.
 *
 * @param path Relative path under the SD mount point.
 * @param callback Function called for each entry.
 * @param user_data Opaque pointer forwarded to the callback.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t sd_dir_list(const char *path, sd_dir_callback_t callback, void *user_data);

/**
 * @brief Count files and subdirectories in a directory.
 *
 * @param path Relative path under the SD mount point.
 * @param file_count Pointer to receive the file count.
 * @param dir_count Pointer to receive the directory count.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t sd_dir_count(const char *path, uint32_t *file_count, uint32_t *dir_count);

/**
 * @brief Recursively copy a directory tree.
 *
 * @param src Source path relative to SD mount point.
 * @param dst Destination path relative to SD mount point.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t sd_dir_copy_recursive(const char *src, const char *dst);

/**
 * @brief Get the total size of all files in a directory tree.
 *
 * @param path Relative path under the SD mount point.
 * @param total_size Pointer to receive the total size in bytes.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t sd_dir_get_size(const char *path, uint64_t *total_size);

#ifdef __cplusplus
}
#endif

#endif // SD_CARD_DIR_H
