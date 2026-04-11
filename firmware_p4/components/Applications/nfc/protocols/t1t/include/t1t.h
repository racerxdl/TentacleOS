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
 * @file t1t.h
 * @brief NFC Forum Type 1 Tag (Topaz) commands.
 */
#ifndef T1T_H
#define T1T_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "highboy_nfc_error.h"
#include "highboy_nfc_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define T1T_UID_LEN  7U
#define T1T_UID4_LEN 4U

/**
 * @brief Type 1 Tag descriptor.
 */
typedef struct {
  uint8_t hr0;              /**< Header ROM byte 0. */
  uint8_t hr1;              /**< Header ROM byte 1. */
  uint8_t uid[T1T_UID_LEN]; /**< Tag UID (4 or 7 bytes). */
  uint8_t uid_len;          /**< Actual UID length (4 or 7). */
  bool is_topaz512;         /**< true if tag is Topaz-512. */
} t1t_tag_t;

/**
 * @brief Configure RF for NFC-A 106 kbps and enable field.
 *
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_IO on communication failure
 */
hb_nfc_err_t t1t_poller_init(void);

/**
 * @brief Check if ATQA matches Topaz (Type 1).
 *
 * @param atqa  2-byte ATQA response.
 * @return true if ATQA indicates a Type 1 Tag.
 */
bool t1t_is_atqa(const uint8_t atqa[2]);

/**
 * @brief Send RID and fill HR0/HR1 + UID0..3.
 *
 * @param[out] tag  Tag descriptor populated on success.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_NO_CARD if no response
 */
hb_nfc_err_t t1t_rid(t1t_tag_t *tag);

/**
 * @brief REQA + RID; validates ATQA=0x0C 0x00.
 *
 * @param[out] tag  Tag descriptor populated on success.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_NO_CARD if no Type 1 Tag found
 */
hb_nfc_err_t t1t_select(t1t_tag_t *tag);

/**
 * @brief Read all memory (RALL). Output includes HR0..RES bytes (no CRC).
 *
 * @param tag          Tag with valid UID.
 * @param[out] out     Output buffer.
 * @param out_max      Output buffer capacity.
 * @param[out] out_len Set to actual bytes read on success.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT on no response
 */
hb_nfc_err_t t1t_rall(t1t_tag_t *tag, uint8_t *out, size_t out_max, size_t *out_len);

/**
 * @brief Read a single byte at address.
 *
 * @param tag       Tag with valid UID.
 * @param addr      Byte address to read.
 * @param[out] data Read byte value on success.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT on no response
 */
hb_nfc_err_t t1t_read_byte(const t1t_tag_t *tag, uint8_t addr, uint8_t *data);

/**
 * @brief Write with erase (WRITE-E).
 *
 * @param tag   Tag with valid UID.
 * @param addr  Byte address to write.
 * @param data  Byte value to write.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT on no response
 */
hb_nfc_err_t t1t_write_e(const t1t_tag_t *tag, uint8_t addr, uint8_t data);

/**
 * @brief Write no erase (WRITE-NE).
 *
 * @param tag   Tag with valid UID.
 * @param addr  Byte address to write.
 * @param data  Byte value to OR with existing data.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT on no response
 */
hb_nfc_err_t t1t_write_ne(const t1t_tag_t *tag, uint8_t addr, uint8_t data);

/**
 * @brief Topaz-512 only: read 8-byte block.
 *
 * @param tag       Tag with valid UID (must be Topaz-512).
 * @param block     Block number to read.
 * @param[out] out  8-byte output buffer.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT on no response
 */
hb_nfc_err_t t1t_read8(const t1t_tag_t *tag, uint8_t block, uint8_t out[8]);

/**
 * @brief Topaz-512 only: write 8-byte block with erase.
 *
 * @param tag    Tag with valid UID (must be Topaz-512).
 * @param block  Block number to write.
 * @param data   8 bytes of data to write.
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_TIMEOUT on no response
 */
hb_nfc_err_t t1t_write_e8(const t1t_tag_t *tag, uint8_t block, const uint8_t data[8]);

#ifdef __cplusplus
}
#endif

#endif /* T1T_H */
