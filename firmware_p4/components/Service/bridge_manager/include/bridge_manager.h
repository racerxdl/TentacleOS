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

#ifndef BRIDGE_MANAGER_H
#define BRIDGE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

/**
 * @brief Synchronize P4 and C5 firmware versions and initialize the bridge.
 *
 * Queries the C5 firmware version over SPI. If it differs from the
 * expected version, triggers an OTA update via c5_flasher.
 *
 * @return ESP_OK on success, ESP_FAIL if synchronization fails.
 */
esp_err_t bridge_manager_init(void);

/**
 * @brief Manually trigger a C5 firmware update.
 *
 * @return ESP_OK on success, or an error code from c5_flasher.
 */
esp_err_t bridge_manager_force_update(void);

#ifdef __cplusplus
}
#endif

#endif // BRIDGE_MANAGER_H
