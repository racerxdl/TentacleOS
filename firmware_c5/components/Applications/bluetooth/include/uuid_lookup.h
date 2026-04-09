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

#ifndef UUID_LOOKUP_H
#define UUID_LOOKUP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "host/ble_uuid.h"

/**
 * @brief Look up a human-readable name for a BLE UUID.
 *
 * @param uuid Pointer to the BLE UUID to look up.
 * @return Name string, or "Unknown Service/Char" if not found.
 */
const char *uuid_get_name(const ble_uuid_t *uuid);

#ifdef __cplusplus
}
#endif

#endif // UUID_LOOKUP_H
