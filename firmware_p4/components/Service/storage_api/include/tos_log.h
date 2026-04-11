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

#ifndef TOS_LOG_H
#define TOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

/**
 * @brief Initialize the persistent log system.
 *
 * Redirects all ESP_LOGx output to a file on the SD card via
 * esp_log_set_vprintf. Logs are written to both serial and file
 * simultaneously.
 *
 * Log files are rotated automatically:
 *   - sys.1.log (current) -> sys.2.log -> ... -> sys.5.log
 *   - Maximum 5 files, total limited to 10 MB
 *   - When sys.1.log exceeds 2 MB, rotation occurs
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL if the log file cannot be opened
 */
esp_err_t tos_log_init(void);

/**
 * @brief Flush and close the log file.
 *
 * Restores the default ESP_LOG vprintf handler. Call this before
 * unmounting the SD card.
 */
void tos_log_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // TOS_LOG_H
