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

#ifndef ST25R3916_CORE_H
#define ST25R3916_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

#include "highboy_nfc.h"

/**
 * @brief Initialize the ST25R3916 core.
 */
esp_err_t st25r3916_core_init(const highboy_nfc_config_t *config);

/**
 * @brief Deinitialize the driver.
 */
void st25r3916_core_deinit(void);

/**
 * @brief Read and validate the chip IC identity.
 */
esp_err_t st25r3916_core_check_id(uint8_t *out_id, uint8_t *out_type, uint8_t *out_rev);

/**
 * @brief Enable the RF field.
 */
esp_err_t st25r3916_core_field_on(void);

/**
 * @brief Disable the RF field.
 */
esp_err_t st25r3916_core_field_off(void);

/**
 * @brief Check if the RF field is active.
 */
bool st25r3916_core_field_is_on(void);

/**
 * @brief Power-cycle the RF field.
 */
esp_err_t st25r3916_core_field_cycle(void);

/**
 * @brief Configure the chip for ISO 14443-A.
 */
esp_err_t st25r3916_core_set_mode_nfca(void);

/**
 * @brief Dump all registers for diagnostics.
 */
void st25r3916_core_dump_regs(void);

#ifdef __cplusplus
}
#endif

#endif // ST25R3916_CORE_H
