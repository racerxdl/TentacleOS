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

#ifndef WIFI_DEAUTHER_H
#define WIFI_DEAUTHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "esp_wifi_types.h"

/**
 * @brief Deauthentication frame type (reason code variant).
 */
typedef enum {
  WIFI_DEAUTHER_TYPE_INVALID_AUTH = 0,
  WIFI_DEAUTHER_TYPE_INACTIVITY,
  WIFI_DEAUTHER_TYPE_CLASS3,
  WIFI_DEAUTHER_TYPE_COUNT
} wifi_deauther_frame_type_t;

/**
 * @brief Send a single deauthentication frame to the AP itself.
 *
 * @param ap_record  Target AP information.
 * @param type       Deauth reason code variant.
 */
void wifi_deauther_send_deauth_frame(const wifi_ap_record_t *ap_record,
                                     wifi_deauther_frame_type_t type);

/**
 * @brief Send a broadcast deauthentication frame on behalf of the AP.
 *
 * @param ap_record  Target AP information.
 * @param type       Deauth reason code variant.
 */
void wifi_deauther_send_broadcast_deauth(const wifi_ap_record_t *ap_record,
                                         wifi_deauther_frame_type_t type);

/**
 * @brief Send a raw 802.11 frame.
 *
 * @param frame_buffer  Pointer to the raw frame data.
 * @param size          Length of the frame in bytes.
 */
void wifi_deauther_send_raw_frame(const uint8_t *frame_buffer, int size);

/**
 * @brief Send an association request to the target AP.
 *
 * @param ap_record  Target AP information.
 */
void wifi_deauther_send_association_request(const wifi_ap_record_t *ap_record);

/**
 * @brief Start a continuous deauthentication attack.
 *
 * @param ap_record     Target AP information.
 * @param type          Deauth reason code variant.
 * @param is_broadcast  If true, send broadcast frames; otherwise target the AP.
 * @return true on success, false on failure.
 */
bool wifi_deauther_start(const wifi_ap_record_t *ap_record,
                         wifi_deauther_frame_type_t type,
                         bool is_broadcast);

/**
 * @brief Start a targeted deauthentication attack against a specific client.
 *
 * @param ap_record   Target AP information.
 * @param client_mac  MAC address of the client to deauthenticate (6 bytes).
 * @param type        Deauth reason code variant.
 * @return true on success, false on failure.
 */
bool wifi_deauther_start_targeted(const wifi_ap_record_t *ap_record,
                                  const uint8_t client_mac[6],
                                  wifi_deauther_frame_type_t type);

/**
 * @brief Stop the deauthentication attack.
 */
void wifi_deauther_stop(void);

/**
 * @brief Check if the deauthentication attack is currently running.
 *
 * @return true if running, false otherwise.
 */
bool wifi_deauther_is_running(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_DEAUTHER_H
