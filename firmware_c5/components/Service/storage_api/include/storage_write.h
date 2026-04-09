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
 * @file storage_write.h
 * @brief Write operations — backend agnostic.
 */

#ifndef STORAGE_WRITE_H
#define STORAGE_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

/**
 * @brief Write a null-terminated string to a file (overwrite).
 *
 * @param path  File path.
 * @param data  String to write.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_string(const char *path, const char *data);

/**
 * @brief Append a null-terminated string to a file.
 *
 * @param path  File path.
 * @param data  String to append.
 * @return ESP_OK on success.
 */
esp_err_t storage_append_string(const char *path, const char *data);

/**
 * @brief Write raw binary data to a file (overwrite).
 *
 * @param path  File path.
 * @param data  Data buffer.
 * @param size  Number of bytes to write.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_binary(const char *path, const void *data, size_t size);

/**
 * @brief Append raw binary data to a file.
 *
 * @param path  File path.
 * @param data  Data buffer.
 * @param size  Number of bytes to append.
 * @return ESP_OK on success.
 */
esp_err_t storage_append_binary(const char *path, const void *data, size_t size);

/**
 * @brief Write a line (with trailing newline) to a file (overwrite).
 *
 * @param path  File path.
 * @param line  Line content without newline.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_line(const char *path, const char *line);

/**
 * @brief Append a line (with trailing newline) to a file.
 *
 * @param path  File path.
 * @param line  Line content without newline.
 * @return ESP_OK on success.
 */
esp_err_t storage_append_line(const char *path, const char *line);

/**
 * @brief Write a printf-formatted string to a file (overwrite).
 *
 * @param path    File path.
 * @param format  printf format string.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_formatted(const char *path, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

/**
 * @brief Append a printf-formatted string to a file.
 *
 * @param path    File path.
 * @param format  printf format string.
 * @return ESP_OK on success.
 */
esp_err_t storage_append_formatted(const char *path, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

/**
 * @brief Write a raw buffer to a file (overwrite).
 *
 * @param path    File path.
 * @param buffer  Data buffer.
 * @param size    Number of bytes.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_buffer(const char *path, const void *buffer, size_t size);

/**
 * @brief Write an array of bytes to a file (overwrite).
 *
 * @param path   File path.
 * @param bytes  Byte array.
 * @param count  Number of bytes.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_bytes(const char *path, const uint8_t *bytes, size_t count);

/**
 * @brief Write a single byte to a file (overwrite).
 *
 * @param path  File path.
 * @param byte  The byte to write.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_byte(const char *path, uint8_t byte);

/**
 * @brief Write a 32-bit integer as a string to a file (overwrite).
 *
 * @param path   File path.
 * @param value  Integer value.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_int(const char *path, int32_t value);

/**
 * @brief Write a float as a string to a file (overwrite).
 *
 * @param path   File path.
 * @param value  Float value.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_float(const char *path, float value);

/**
 * @brief Write a CSV row to a file (overwrite).
 *
 * @param path         File path.
 * @param columns      Array of column strings.
 * @param num_columns  Number of columns.
 * @return ESP_OK on success.
 */
esp_err_t storage_write_csv_row(const char *path, const char **columns, size_t num_columns);

/**
 * @brief Append a CSV row to a file.
 *
 * @param path         File path.
 * @param columns      Array of column strings.
 * @param num_columns  Number of columns.
 * @return ESP_OK on success.
 */
esp_err_t storage_append_csv_row(const char *path, const char **columns, size_t num_columns);

#ifdef __cplusplus
}
#endif

#endif