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

#ifndef SIGNAL_MONITOR_H
#define SIGNAL_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Start monitoring the signal strength of a specific BSSID.
 *
 * @param bssid    BSSID to monitor (6 bytes).
 * @param channel  Wi-Fi channel to listen on.
 */
void signal_monitor_start(const uint8_t *bssid, uint8_t channel);

/**
 * @brief Stop the signal monitor and free resources.
 */
void signal_monitor_stop(void);

/**
 * @brief Get the last observed RSSI value.
 *
 * @return RSSI in dBm, or -127 if no signal detected.
 */
int8_t signal_monitor_get_rssi(void);

/**
 * @brief Get time elapsed since the last packet was seen.
 *
 * @return Milliseconds since last packet, or UINT32_MAX if never seen.
 */
uint32_t signal_monitor_get_last_seen_ms(void);

#ifdef __cplusplus
}
#endif

#endif // SIGNAL_MONITOR_H
