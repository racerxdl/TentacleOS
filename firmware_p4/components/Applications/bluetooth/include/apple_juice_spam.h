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

#ifndef APPLE_JUICE_SPAM_H
#define APPLE_JUICE_SPAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Generate an Apple BLE proximity pairing spam payload.
 *
 * @param buffer  Output buffer for the payload. Must not be NULL.
 * @param max_len Maximum size of the output buffer.
 * @return Number of bytes written, or 0 on failure.
 */
int generate_apple_juice_payload(uint8_t *buffer, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif // APPLE_JUICE_SPAM_H
