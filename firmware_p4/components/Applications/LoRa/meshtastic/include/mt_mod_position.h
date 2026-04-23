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

#ifndef MT_MOD_POSITION_H
#define MT_MOD_POSITION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mt_modules.h"

/**
 * @brief Initialize the PositionModule.
 *
 * Loads fixed_position (lat/lon/alt) from NVS if present.
 */
void mt_mod_position_init(uint32_t node_num);

/**
 * @brief Handle an incoming Position protobuf from another node.
 *
 * Replies with our fixed_position if want_response is set and
 * fixed_position is configured.
 */
void mt_mod_position_on_received(const mt_packet_meta_t *meta,
                                  const uint8_t *payload, uint16_t len);

/**
 * @brief Periodic tick. Broadcasts position every 15 min if fixed.
 */
void mt_mod_position_tick(uint32_t now_s);

/**
 * @brief Set a fixed position (and persist to NVS).
 *
 * @param lat_e7  Latitude in degrees * 1e7.
 * @param lon_e7  Longitude in degrees * 1e7.
 * @param alt_m   Altitude in meters.
 */
void mt_mod_position_set_fixed(int32_t lat_e7, int32_t lon_e7, int32_t alt_m);

/**
 * @brief Remove the fixed position (and delete from NVS).
 */
void mt_mod_position_remove_fixed(void);

#ifdef __cplusplus
}
#endif

#endif // MT_MOD_POSITION_H
