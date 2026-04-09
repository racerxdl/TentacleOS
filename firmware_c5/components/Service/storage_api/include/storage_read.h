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
 * @file storage_read.h
 * @brief Read operations — backend agnostic.
 */

#ifndef STORAGE_READ_H
#define STORAGE_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

/**
 * @brief Read a file as a null-terminated string.
 *
 * @param path         File path.
 * @param[out] buffer  Destination buffer.
 * @param buffer_size  Size of the buffer including the null terminator.
 * @return ESP_OK on success.
 */
esp_err_t storage_read_string(const char *path, char *buffer, size_t buffer_size);

/**
 * @brief Read raw binary data from a file.
 *
 * @param path          File path.
 * @param[out] buffer   Destination buffer.
 * @param size          Maximum bytes to read.
 * @param[out] bytes_read  Actual bytes read (may be NULL).
 * @return ESP_OK on success.
 */
esp_err_t storage_read_binary(const char *path, void *buffer, size_t size, size_t *bytes_read);

typedef void (*storage_line_callback_t)(const char *line, void *user_data);

/**
 * @brief Read a specific line from a file (1-based).
 *
 * @param path         File path.
 * @param[out] buffer  Destination buffer.
 * @param buffer_size  Size of the buffer.
 * @param line_number  1-based line index.
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if line does not exist.
 */
esp_err_t
storage_read_line(const char *path, char *buffer, size_t buffer_size, uint32_t line_number);

/**
 * @brief Read the first line of a file.
 *
 * @param path         File path.
 * @param[out] buffer  Destination buffer.
 * @param buffer_size  Size of the buffer.
 * @return ESP_OK on success.
 */
esp_err_t storage_read_first_line(const char *path, char *buffer, size_t buffer_size);

/**
 * @brief Read the last line of a file.
 *
 * @param path         File path.
 * @param[out] buffer  Destination buffer.
 * @param buffer_size  Size of the buffer.
 * @return ESP_OK on success.
 */
esp_err_t storage_read_last_line(const char *path, char *buffer, size_t buffer_size);

/**
 * @brief Iterate over every line in a file via a callback.
 *
 * @param path       File path.
 * @param callback   Called once per line.
 * @param user_data  Opaque context forwarded to the callback.
 * @return ESP_OK on success.
 */
esp_err_t storage_read_lines(const char *path, storage_line_callback_t callback, void *user_data);

/**
 * @brief Count the number of lines in a file.
 *
 * @param path           File path.
 * @param[out] line_count  Number of newline characters found.
 * @return ESP_OK on success.
 */
esp_err_t storage_count_lines(const char *path, uint32_t *line_count);

/**
 * @brief Read a chunk of a file starting at a byte offset.
 *
 * @param path          File path.
 * @param offset        Byte offset to start reading.
 * @param[out] buffer   Destination buffer.
 * @param size          Maximum bytes to read.
 * @param[out] bytes_read  Actual bytes read (may be NULL).
 * @return ESP_OK on success.
 */
esp_err_t
storage_read_chunk(const char *path, size_t offset, void *buffer, size_t size, size_t *bytes_read);

/**
 * @brief Read a file and parse its contents as a 32-bit integer.
 *
 * @param path       File path.
 * @param[out] value  Parsed integer.
 * @return ESP_OK on success.
 */
esp_err_t storage_read_int(const char *path, int32_t *value);

/**
 * @brief Read a file and parse its contents as a float.
 *
 * @param path       File path.
 * @param[out] value  Parsed float.
 * @return ESP_OK on success.
 */
esp_err_t storage_read_float(const char *path, float *value);

/**
 * @brief Read raw bytes from a file.
 *
 * @param path        File path.
 * @param[out] bytes  Destination buffer.
 * @param max_count   Maximum bytes to read.
 * @param[out] count  Actual bytes read (may be NULL).
 * @return ESP_OK on success.
 */
esp_err_t storage_read_bytes(const char *path, uint8_t *bytes, size_t max_count, size_t *count);

/**
 * @brief Read a single byte from a file.
 *
 * @param path       File path.
 * @param[out] byte  The byte read.
 * @return ESP_OK on success.
 */
esp_err_t storage_read_byte(const char *path, uint8_t *byte);

/**
 * @brief Check if a file contains a given substring.
 *
 * @param path        File path.
 * @param search      Substring to look for.
 * @param[out] found  Set to true if the substring is present.
 * @return ESP_OK on success.
 */
esp_err_t storage_file_contains(const char *path, const char *search, bool *found);

/**
 * @brief Count the number of occurrences of a substring in a file.
 *
 * @param path       File path.
 * @param search     Substring to count.
 * @param[out] count  Number of occurrences.
 * @return ESP_OK on success.
 */
esp_err_t storage_count_occurrences(const char *path, const char *search, uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif