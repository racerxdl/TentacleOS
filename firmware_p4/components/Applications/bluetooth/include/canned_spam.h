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

#ifndef CANNED_SPAM_H
#define CANNED_SPAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

/**
 * @brief Descriptor for a BLE spam attack type.
 */
typedef struct {
  const char *name;
} canned_spam_type_t;

/**
 * @brief Get the number of available spam attack types.
 *
 * @return Number of attack types.
 */
int spam_get_attack_count(void);

/**
 * @brief Get the descriptor for a spam attack type by index.
 *
 * @param index Zero-based attack index.
 * @return Pointer to the attack type descriptor, or NULL if index is out of range.
 */
const canned_spam_type_t *spam_get_attack_type(int index);

/**
 * @brief Start a BLE spam attack.
 *
 * @param attack_index Zero-based index of the attack type to start.
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_STATE if already running
 *   - ESP_FAIL if bluetooth service is not running
 *   - ESP_ERR_NOT_FOUND if attack_index is invalid
 */
esp_err_t spam_start(int attack_index);

/**
 * @brief Stop the currently running BLE spam attack.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_STATE if not running
 */
esp_err_t spam_stop(void);

#ifdef __cplusplus
}
#endif

#endif // CANNED_SPAM_H
