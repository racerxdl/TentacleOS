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

#ifndef MT_MOD_ROUTING_H
#define MT_MOD_ROUTING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mt_modules.h"

/**
 * @brief Initialize the RoutingModule.
 */
void mt_mod_routing_init(uint32_t node_num);

/**
 * @brief Handle an incoming Routing protobuf (ACK/NAK).
 *
 * Currently logs only. ACK/NAK parsing to update retransmit state is
 * deferred to Fase 2 (ReliableRouter).
 */
void mt_mod_routing_on_received(const mt_packet_meta_t *meta, const uint8_t *payload, uint16_t len);

/**
 * @brief Emit an ACK in response to a packet that had want_ack set.
 *
 * Builds Routing { error_reason = NONE } and enqueues it for TX at
 * priority MESH_PRIO_ACK. Target is meta->from, with meta->id carried
 * as request_id.
 */
void mt_mod_routing_maybe_send_ack(const mt_packet_meta_t *meta);

#ifdef __cplusplus
}
#endif

#endif // MT_MOD_ROUTING_H
