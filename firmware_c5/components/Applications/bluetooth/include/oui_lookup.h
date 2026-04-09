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

#ifndef OUI_LOOKUP_H
#define OUI_LOOKUP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Look up the vendor name for a MAC address using the OUI prefix.
 *
 * @param mac Pointer to the MAC address (at least 3 bytes). Must not be NULL.
 * @return Vendor name string, or "Unknown" if not found.
 */
const char *oui_get_vendor(const uint8_t *mac);

#ifdef __cplusplus
}
#endif

#endif // OUI_LOOKUP_H
