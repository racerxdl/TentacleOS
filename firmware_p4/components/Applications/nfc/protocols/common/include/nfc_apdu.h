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
 * @file nfc_apdu.h
 * @brief ISO/IEC 7816-4 APDU helpers over ISO14443-4 (T=CL).
 */
#ifndef NFC_APDU_H
#define NFC_APDU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "highboy_nfc_types.h"
#include "highboy_nfc_error.h"
#include "highboy_nfc_compat.h"

/* ---- APDU class byte ---- */
#define APDU_CLA_ISO7816 0x00U

/* ---- APDU instruction codes (ISO 7816-4) ---- */
#define APDU_INS_SELECT        0xA4U
#define APDU_INS_READ_BINARY   0xB0U
#define APDU_INS_UPDATE_BINARY 0xD6U
#define APDU_INS_GET_RESPONSE  0xC0U
#define APDU_INS_VERIFY        0x20U
#define APDU_INS_GET_DATA      0xCAU

/* ---- APDU status words ---- */
#define APDU_SW_OK             0x9000U
#define APDU_SW_MORE_DATA      0x6100U /**< SW1=61: more data available. */
#define APDU_SW_WRONG_LENGTH   0x6700U
#define APDU_SW_SECURITY       0x6982U
#define APDU_SW_CONDITIONS     0x6985U /**< Conditions of use not satisfied. */
#define APDU_SW_AUTH_FAILED    0x6300U
#define APDU_SW_WRONG_P1P2     0x6A86U
#define APDU_SW_FILE_NOT_FOUND 0x6A82U
#define APDU_SW_WRONG_OFFSET   0x6B00U
#define APDU_SW_WRONG_LE       0x6C00U /**< SW1=6C: wrong Le, SW2=exact Le. */
#define APDU_SW_INS_NOT_SUP    0x6D00U
#define APDU_SW_CLA_NOT_SUP    0x6E00U
#define APDU_SW_UNKNOWN        0x6F00U

/* ---- APDU status word masks ---- */
#define APDU_SW_CLASS_MASK 0xFF00U /**< Mask for SW1 byte. */
#define APDU_SW_DATA_MASK  0x00FFU /**< Mask for SW2 byte. */

/* ---- APDU protocol constants ---- */
#define APDU_HEADER_SIZE     5U    /**< CLA + INS + P1 + P2 + Lc/Le. */
#define APDU_LE_MAX          0xFFU /**< Maximum short-form Le value. */
#define APDU_MAX_GET_RESP    8     /**< Max GET RESPONSE chaining retries. */
#define APDU_FCI_TAG_DF_NAME 0x84U /**< FCI template: DF name tag. */

/**
 * @brief APDU output buffer descriptor.
 *
 * Groups a destination buffer pointer and its capacity for APDU build helpers.
 */
typedef struct {
  uint8_t *buf; /**< Destination buffer. */
  size_t max;   /**< Buffer capacity in bytes. */
} nfc_apdu_buf_t;

/**
 * @brief Read-only data span for APDU payloads (AID, PIN, etc.).
 */
typedef struct {
  const uint8_t *ptr; /**< Data pointer. */
  size_t len;         /**< Data length in bytes. */
} nfc_apdu_data_t;

/**
 * @brief APDU channel descriptor for ISO14443-4 transceive operations.
 *
 * Groups transport-level parameters shared across all APDU exchange calls.
 */
typedef struct {
  hb_nfc_protocol_t proto; /**< HB_PROTO_ISO14443_4A or HB_PROTO_ISO14443_4B. */
  const void *ctx;         /**< Protocol context (nfc_iso_dep_data_t* or nfc_iso14443b_data_t*). */
  int timeout_ms;          /**< Timeout in milliseconds. */
} nfc_apdu_channel_t;

/**
 * @brief APDU response descriptor.
 *
 * Groups output buffer, length, and status word for APDU responses.
 */
typedef struct {
  uint8_t *buf; /**< Response data buffer. */
  size_t max;   /**< Buffer capacity in bytes. */
  size_t len;   /**< Actual response data length (excluding SW). */
  uint16_t sw;  /**< ISO 7816-4 status word. */
} nfc_apdu_resp_t;

/**
 * @brief FCI (File Control Information) parsed data.
 */
typedef struct {
  uint8_t df_name[16];
  uint8_t df_name_len;
} nfc_fci_info_t;

/**
 * @brief Translate a status word to a human-readable string.
 *
 * @param sw  ISO 7816-4 status word.
 * @return Pointer to a static string describing the status word.
 */
const char *nfc_apdu_sw_str(uint16_t sw);

/**
 * @brief Map a status word to a coarse NFC error code.
 *
 * @param sw  ISO 7816-4 status word.
 * @return
 *   - HB_NFC_OK if sw is APDU_SW_OK
 *   - HB_NFC_ERR_AUTH on authentication/security failures
 *   - HB_NFC_ERR on other error status words
 */
hb_nfc_err_t nfc_apdu_sw_to_err(uint16_t sw);

/**
 * @brief Transceive a generic APDU over ISO14443-4A or 4B.
 *
 * @param ch         Channel descriptor (protocol, context, timeout).
 * @param apdu       Command APDU bytes.
 * @param apdu_len   Length of the command APDU.
 * @param[out] resp  Response descriptor (buffer, length, status word).
 * @return
 *   - HB_NFC_OK on SW=0x9000
 *   - Mapped error code otherwise
 */
hb_nfc_err_t nfc_apdu_transceive(const nfc_apdu_channel_t *ch,
                                 const uint8_t *apdu,
                                 size_t apdu_len,
                                 nfc_apdu_resp_t *resp);

/**
 * @brief Send an APDU command and receive the response.
 *
 * Convenience wrapper around nfc_apdu_transceive.
 *
 * @param ch         Channel descriptor (protocol, context, timeout).
 * @param apdu       Command APDU bytes.
 * @param apdu_len   Length of the command APDU.
 * @param[out] resp  Response descriptor (buffer, length, status word).
 * @return
 *   - HB_NFC_OK on success
 *   - Mapped error code on failure
 */
hb_nfc_err_t nfc_apdu_send(const nfc_apdu_channel_t *ch,
                           const uint8_t *apdu,
                           size_t apdu_len,
                           nfc_apdu_resp_t *resp);

/**
 * @brief Receive an APDU response for a previously sent command.
 *
 * Convenience wrapper around nfc_apdu_transceive.
 *
 * @param ch         Channel descriptor (protocol, context, timeout).
 * @param apdu       Command APDU bytes.
 * @param apdu_len   Length of the command APDU.
 * @param[out] resp  Response descriptor (buffer, length, status word).
 * @return
 *   - HB_NFC_OK on success
 *   - Mapped error code on failure
 */
hb_nfc_err_t nfc_apdu_recv(const nfc_apdu_channel_t *ch,
                           const uint8_t *apdu,
                           size_t apdu_len,
                           nfc_apdu_resp_t *resp);

/**
 * @brief Build a SELECT AID APDU.
 *
 * @param dst        Output buffer descriptor.
 * @param aid        Application identifier data span.
 * @param le_present Whether to include the Le byte.
 * @param le         Expected response length byte.
 * @return Number of bytes written to the output buffer.
 */
size_t
nfc_apdu_build_select_aid(nfc_apdu_buf_t dst, nfc_apdu_data_t aid, bool le_present, uint8_t le);

/**
 * @brief Build a SELECT FILE by file ID APDU.
 *
 * @param dst        Output buffer descriptor.
 * @param fid        File identifier.
 * @param le_present Whether to include the Le byte.
 * @param le         Expected response length byte.
 * @return Number of bytes written to the output buffer.
 */
size_t nfc_apdu_build_select_file(nfc_apdu_buf_t dst, uint16_t fid, bool le_present, uint8_t le);

/**
 * @brief Build a READ BINARY APDU.
 *
 * @param dst     Output buffer descriptor.
 * @param offset  Byte offset to read from.
 * @param le      Expected response length byte.
 * @return Number of bytes written to the output buffer.
 */
size_t nfc_apdu_build_read_binary(nfc_apdu_buf_t dst, uint16_t offset, uint8_t le);

/**
 * @brief Build an UPDATE BINARY APDU.
 *
 * @param dst        Output buffer descriptor.
 * @param offset     Byte offset to write to.
 * @param data       Data span to write.
 * @return Number of bytes written to the output buffer.
 */
size_t nfc_apdu_build_update_binary(nfc_apdu_buf_t dst, uint16_t offset, nfc_apdu_data_t data);

/**
 * @brief Build a VERIFY APDU for PIN or password authentication.
 *
 * @param dst        Output buffer descriptor.
 * @param p1         P1 parameter.
 * @param p2         P2 parameter (key reference).
 * @param data       Verification data span (PIN/password).
 * @return Number of bytes written to the output buffer.
 */
size_t nfc_apdu_build_verify(nfc_apdu_buf_t dst, uint8_t p1, uint8_t p2, nfc_apdu_data_t data);

/**
 * @brief Build a GET DATA APDU.
 *
 * @param dst   Output buffer descriptor.
 * @param p1    P1 tag identifier.
 * @param p2    P2 tag identifier.
 * @param le    Expected response length byte.
 * @return Number of bytes written to the output buffer.
 */
size_t nfc_apdu_build_get_data(nfc_apdu_buf_t dst, uint8_t p1, uint8_t p2, uint8_t le);

/**
 * @brief Parse an FCI template and extract the DF name (AID) if present.
 *
 * @param fci      FCI data bytes.
 * @param fci_len  Length of the FCI data.
 * @param[out] out     Parsed FCI information.
 * @return true if the FCI was parsed successfully, false otherwise.
 */
bool nfc_apdu_parse_fci(const uint8_t *fci, size_t fci_len, nfc_fci_info_t *out);

#ifdef __cplusplus
}
#endif

#endif /* NFC_APDU_H */
