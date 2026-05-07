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

#ifndef MESHCORE_BLE_H
#define MESHCORE_BLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

#include "meshcore.h"

/**
 * @brief BLE companion configuration.
 *
 * `device_name` is used as a prefix in advertising. The final name is
 * "<prefix>-XXXX" where XXXX is the last 4 chars of the MAC.
 */
typedef struct {
  const char *device_name;
} meshcore_ble_config_t;

/**
 * @brief Initializes NimBLE + Nordic UART Service + companion protocol handlers.
 *
 * Must be called AFTER meshcore_init (depends on loaded identity).
 * Configures SC + MITM + DISP_ONLY with PIN persisted in NVS (randomly
 * generated on first boot). Preferred MTU = 512.
 *
 * @param cfg  Configuration. Caller retains ownership.
 * @return
 *   - ESP_OK on success
 *   - ESP_ERR_INVALID_ARG if cfg is NULL
 *   - ESP_FAIL on NimBLE stack failure
 */
esp_err_t meshcore_ble_init(const meshcore_ble_config_t *cfg);

/**
 * @brief Checks whether a BLE client is connected.
 *
 * @return true if connected, false otherwise.
 */
bool meshcore_ble_is_connected(void);

/**
 * @brief Sends a raw frame via the TX characteristic (notify).
 *
 * @param data  Buffer.
 * @param len   Size.
 */
void meshcore_ble_send(const uint8_t *data, uint16_t len);

/**
 * @brief Push of a newly received ADVERT (PUSH_CODE_NEW_ADVERT = 0x8A).
 *
 * @param contact  Newly announced contact.
 */
void meshcore_ble_push_new_advert(const meshcore_contact_t *contact);

/**
 * @brief Push of a channel message (RESP_CODE_CHANNEL_MSG_RECV V1/V3).
 *
 * Stacks into the offline queue; emits tickle 0x83 for the app to poll SYNC.
 *
 * @param channel_idx  Channel slot.
 * @param path_len     Hops.
 * @param timestamp    Unix ts.
 * @param snr          SNR in dB.
 * @param text         Null-terminated UTF-8 text.
 */
void meshcore_ble_push_channel_msg(
    uint8_t channel_idx, uint8_t path_len, uint32_t timestamp, int8_t snr, const char *text);

/**
 * @brief Push of a received DM (RESP_CODE_CONTACT_MSG_RECV V1/V3).
 *
 * @param peer_pub_key  Sender pubkey (32B).
 * @param path_len      Hops.
 * @param txt_type      Sub-type (MC_TXT_TYPE_*).
 * @param timestamp     Unix ts.
 * @param snr           SNR in dB.
 * @param text          UTF-8 text.
 */
void meshcore_ble_push_contact_msg(const uint8_t peer_pub_key[32],
                                   uint8_t path_len,
                                   uint8_t txt_type,
                                   uint32_t timestamp,
                                   int8_t snr,
                                   const char *text);

/**
 * @brief Push of SEND_CONFIRMED (0x82) when ACK arrives for a sent DM.
 *
 * @param ack_crc  ACK CRC.
 * @param snr      SNR of the ACK packet.
 */
void meshcore_ble_push_send_confirmed(uint32_t ack_crc, uint8_t snr);

/**
 * @brief Push of PATH_UPDATED (0x81) when we learn out_path for a contact.
 *
 * @param contact  Contact with freshly updated path.
 */
void meshcore_ble_push_path_updated(const meshcore_contact_t *contact);

#ifdef __cplusplus
}
#endif

#endif // MESHCORE_BLE_H
