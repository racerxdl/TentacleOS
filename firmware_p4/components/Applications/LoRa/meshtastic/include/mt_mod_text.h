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

#ifndef MT_MOD_TEXT_H
#define MT_MOD_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "mt_modules.h"

/**
 * @brief Initialize the TextMessageModule.
 *
 * Clears the 50-slot dedup ring and records our node number.
 */
void mt_mod_text_init(uint32_t node_num);

/**
 * @brief Handle an incoming text message.
 *
 * Deduplicates on packet id (ring buffer of the last 50 ids).
 * Logs the message. Forwarding to the app happens in the mesh core via
 * PhoneAPI; this handler does not push frames directly.
 *
 * @param meta        Packet metadata.
 * @param payload     UTF-8 bytes (or Unishox2 if compressed).
 * @param len         Payload length.
 * @param is_compressed True if the packet arrived on portnum 7 (compressed).
 */
void mt_mod_text_on_received(const mt_packet_meta_t *meta,
                              const uint8_t *payload, uint16_t len,
                              bool is_compressed);

#ifdef __cplusplus
}
#endif

#endif // MT_MOD_TEXT_H
