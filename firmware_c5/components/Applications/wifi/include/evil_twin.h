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

#ifndef EVIL_TWIN_H
#define EVIL_TWIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Start an evil twin attack using the default captive portal template.
 *
 * @param ssid  SSID to broadcast for the rogue access point.
 */
void evil_twin_start_attack(const char *ssid);

/**
 * @brief Start an evil twin attack with a custom captive portal template.
 *
 * @param ssid           SSID to broadcast for the rogue access point.
 * @param template_path  Path to the custom HTML template file.
 */
void evil_twin_start_attack_with_template(const char *ssid, const char *template_path);

/**
 * @brief Stop the evil twin attack and release resources.
 */
void evil_twin_stop_attack(void);

/**
 * @brief Reset the captured password state.
 */
void evil_twin_reset_capture(void);

/**
 * @brief Check if a password has been captured.
 *
 * @return true if a password is available, false otherwise.
 */
bool evil_twin_has_password(void);

/**
 * @brief Get the last captured password.
 *
 * @param[out] out  Buffer to store the password string.
 * @param len       Size of the output buffer.
 */
void evil_twin_get_last_password(char *out, size_t len);

/**
 * @brief Begin receiving a captive portal template over chunked transfer.
 *
 * @param total_size  Total size of the template in bytes.
 */
void evil_twin_tmpl_begin(uint16_t total_size);

/**
 * @brief Receive a chunk of the captive portal template.
 *
 * @param data  Pointer to the chunk data.
 * @param len   Length of the chunk in bytes.
 */
void evil_twin_tmpl_chunk(const uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif // EVIL_TWIN_H
