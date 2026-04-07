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

#ifndef IR_FILE_H
#define IR_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir.h"

/** @brief Maximum length of a signal name, including null terminator. */
#define IR_FILE_NAME_MAX 32

/** @brief Maximum length of a protocol name string, including null terminator. */
#define IR_FILE_PROTO_NAME_MAX 32

/** @brief Maximum line buffer size used during file parsing. */
#define IR_FILE_LINE_BUF_SIZE 1024

/** @brief Initial capacity of the signals array before any growth. */
#define IR_FILE_INITIAL_CAP 8

/**
 * @brief A single IR signal, either decoded or raw.
 *
 * Buffer ownership: raw is allocated on parse or ir_file_add_raw, freed by ir_file_free.
 */
typedef struct {
  char name[IR_FILE_NAME_MAX];
  bool is_raw;
  ir_data_t data;
  uint32_t frequency;
  uint32_t *raw;
  size_t raw_count;
} ir_signal_t;

/**
 * @brief Container for a collection of IR signals parsed from or written to a file.
 *
 * Buffer ownership: signals is grown on demand and freed by ir_file_free.
 */
typedef struct {
  ir_signal_t *signals;
  size_t count;
  size_t capacity;
} ir_file_t;

/**
 * @brief Configuration for ir_file_add_raw.
 */
typedef struct {
  const char *name;
  const rmt_symbol_word_t *symbols;
  size_t count;
  uint32_t freq;
} ir_file_add_raw_cfg_t;

/**
 * @brief Initialize an ir_file_t to an empty state.
 *
 * @param[out] file  File to initialize. Must not be NULL.
 */
void ir_file_init(ir_file_t *file);

/**
 * @brief Free all signals and internal buffers owned by the file.
 *
 * @param[in,out] file  File to free. Must not be NULL.
 */
void ir_file_free(ir_file_t *file);

/**
 * @brief Parse a Flipper Zero IR file from a null-terminated string.
 *
 * Appends parsed signals to @p file. Caller must call ir_file_init before
 * the first parse and ir_file_free when done.
 *
 * @param[in]     content  Null-terminated file content. Must not be NULL.
 * @param[in,out] file     Destination file. Must not be NULL.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_ARG if any pointer is NULL
 *   - ESP_ERR_NO_MEM if allocation fails
 */
esp_err_t ir_file_parse(const char *content, ir_file_t *file);

/**
 * @brief Serialize all signals in the file to a Flipper Zero IR format string.
 *
 * Writes into @p buf up to @p buf_size bytes. Output is truncated if the
 * buffer is too small — no null terminator is guaranteed beyond @p buf_size.
 *
 * @param[in]  file      File to serialize. Must not be NULL.
 * @param[out] buf       Destination buffer. Must not be NULL.
 * @param[in]  buf_size  Capacity of @p buf in bytes. Must be greater than 0.
 *
 * @return Number of bytes written (excluding null terminator).
 */
size_t ir_file_to_string(const ir_file_t *file, char *buf, size_t buf_size);

/**
 * @brief Find a signal by name.
 *
 * @param[in] file  File to search. Must not be NULL.
 * @param[in] name  Signal name to look up. Must not be NULL.
 *
 * @return Pointer to the matching signal, or NULL if not found.
 */
ir_signal_t *ir_file_find(const ir_file_t *file, const char *name);

/**
 * @brief Transmit a signal over IR.
 *
 * Dispatches to ir_send_raw for raw signals or ir_send for decoded signals.
 *
 * @param[in] signal  Signal to transmit. Must not be NULL.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_ARG if signal is NULL
 *   - Other ESP_ERR codes from the IR driver
 */
esp_err_t ir_file_send(const ir_signal_t *signal);

/**
 * @brief Add a decoded IR signal to the file.
 *
 * @param[in,out] file  Destination file. Must not be NULL.
 * @param[in]     name  Signal name. Must not be NULL.
 * @param[in]     data  Decoded IR data to store. Must not be NULL.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_ARG if any pointer is NULL
 *   - ESP_ERR_NO_MEM if allocation fails
 */
esp_err_t ir_file_add_parsed(ir_file_t *file, const char *name, const ir_data_t *data);

/**
 * @brief Add a raw RMT symbol sequence to the file.
 *
 * @param[in,out] file  Destination file. Must not be NULL.
 * @param[in]     cfg   Add configuration. Must not be NULL. All fields must be valid.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_ARG if any pointer is NULL or cfg->count is 0
 *   - ESP_ERR_NO_MEM if allocation fails
 */
esp_err_t ir_file_add_raw(ir_file_t *file, const ir_file_add_raw_cfg_t *cfg);

#ifdef __cplusplus
}
#endif

#endif // IR_FILE_H