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
/**
 * @file poller.h
 * @brief ISO14443A Poller REQA/WUPA, anti-collision, SELECT.
 */
#ifndef ISO14443A_POLLER_H
#define ISO14443A_POLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "highboy_nfc_types.h"
#include "highboy_nfc_error.h"
#include "highboy_nfc_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Send REQA or WUPA and get ATQA.
 *
 * Tries REQA first; if no response, waits 5 ms and sends WUPA.
 *
 * @param[out] atqa  2-byte ATQA response.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT if no tag responds
 */
hb_nfc_err_t iso14443a_poller_activate(uint8_t atqa[2]);

/**
 * @brief Full card activation: REQA, anti-collision, SELECT (all cascade levels).
 *
 * @param[out] card  Populated with UID, ATQA, and SAK.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT if no tag responds
 */
hb_nfc_err_t iso14443a_poller_select(nfc_iso14443a_data_t *card);

/**
 * @brief Re-select a card using WUPA, anti-collision, and SELECT.
 *
 * @param[out] card  Populated with UID, ATQA, and SAK.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT if no tag responds
 */
hb_nfc_err_t iso14443a_poller_reselect(nfc_iso14443a_data_t *card);

/**
 * @brief Send HLTA command to halt the tag.
 *
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_IO on failure
 */
hb_nfc_err_t iso14443a_poller_halt(void);

/**
 * @brief Discover multiple NFC-A tags by iterating REQA, SELECT, and HLTA.
 *
 * @param[out] out    Array for discovered cards.
 * @param max_cards   Maximum number of cards to discover.
 * @return Number of cards found (up to max_cards).
 */
int iso14443a_poller_select_all(nfc_iso14443a_data_t *out, size_t max_cards);

/**
 * @brief Send REQA only.
 *
 * @param[out] atqa  2-byte ATQA response.
 * @return 2 on success (ATQA bytes), or 0 on failure.
 */
int iso14443a_poller_reqa(uint8_t atqa[2]);

/**
 * @brief Send WUPA only.
 *
 * @param[out] atqa  2-byte ATQA response.
 * @return 2 on success (ATQA bytes), or 0 on failure.
 */
int iso14443a_poller_wupa(uint8_t atqa[2]);

/**
 * @brief Anti-collision for one cascade level.
 *
 * @param sel         SEL_CL1 (0x93), SEL_CL2 (0x95), or SEL_CL3 (0x97).
 * @param[out] uid_cl Output: 4 UID bytes + BCC (5 bytes total).
 * @return 5 on success, or 0 on failure.
 */
int iso14443a_poller_anticoll(uint8_t sel, uint8_t uid_cl[5]);

/**
 * @brief SELECT for one cascade level.
 *
 * @param sel         SEL_CL1, SEL_CL2, or SEL_CL3.
 * @param uid_cl      5 bytes from anti-collision.
 * @param[out] sak    SAK byte output.
 * @return 1 on success, or 0 on failure.
 */
int iso14443a_poller_sel(uint8_t sel, const uint8_t uid_cl[5], uint8_t *sak);

#ifdef __cplusplus
}
#endif

#endif /* ISO14443A_POLLER_H */
