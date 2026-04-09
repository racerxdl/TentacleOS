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

#ifndef HIGHBOY_NFC_TYPES_H
#define HIGHBOY_NFC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** @name Capacity Constants */
/**@{*/
#define HIGHBOY_NFC_UID_MAX_LEN  10  /**< Max UID (Triple Cascade). */
#define HIGHBOY_NFC_ATS_MAX_LEN  64  /**< Max ATS per ISO 14443-4. */
#define HIGHBOY_NFC_FIFO_MAX_LEN 512 /**< ST25R3916 hardware FIFO depth. */
/**@}*/

/**
 * @brief ISO 14443-A anti-collision data.
 */
typedef struct {
  uint8_t uid[HIGHBOY_NFC_UID_MAX_LEN];
  uint8_t uid_len;
  uint8_t atqa[2];
  uint8_t sak;
} highboy_nfc_iso14443a_t;

/**
 * @brief ISO 14443-4 (ISO-DEP) activation data.
 */
typedef struct {
  uint8_t ats[HIGHBOY_NFC_ATS_MAX_LEN];
  uint16_t ats_len;
  uint8_t fsc;
  uint8_t fwi;
} highboy_nfc_iso_dep_t;

/**
 * @brief ISO 14443-B activation data.
 */
typedef struct {
  uint8_t pupi[4];
  uint8_t app_data[4];
  uint8_t prot_info[3];
} highboy_nfc_iso14443b_t;

/**
 * @brief ISO 15693 (NFC-V) activation data.
 */
typedef struct {
  uint8_t uid[8];
  uint8_t dsfid;
  uint16_t block_count;
  uint8_t block_size;
} highboy_nfc_iso15693_t;

/**
 * @brief MIFARE Classic authentication key types.
 */
typedef enum {
  HIGHBOY_NFC_MF_KEY_A = 0x60,
  HIGHBOY_NFC_MF_KEY_B = 0x61,
} highboy_nfc_mf_key_type_t;

/**
 * @brief NFC protocol identifier.
 */
typedef enum {
  HIGHBOY_NFC_PROTO_ISO14443_3A = 0,
  HIGHBOY_NFC_PROTO_ISO14443_3B,
  HIGHBOY_NFC_PROTO_ISO14443_4A,
  HIGHBOY_NFC_PROTO_ISO14443_4B,
  HIGHBOY_NFC_PROTO_FELICA,
  HIGHBOY_NFC_PROTO_ISO15693,
  HIGHBOY_NFC_PROTO_MF_CLASSIC,
  HIGHBOY_NFC_PROTO_MF_ULTRALIGHT,
  HIGHBOY_NFC_PROTO_MF_PLUS,
  HIGHBOY_NFC_PROTO_MF_DESFIRE,
  HIGHBOY_NFC_PROTO_ST25TB,
  HIGHBOY_NFC_PROTO_SLIX,
  HIGHBOY_NFC_PROTO_COUNT,
  HIGHBOY_NFC_PROTO_UNKNOWN = 0xFF
} highboy_nfc_protocol_t;

/**
 * @brief Combined card descriptor.
 */
typedef struct {
  highboy_nfc_protocol_t protocol;
  highboy_nfc_iso14443a_t iso14443a; /** Always valid for Type A tags. */
  union {
    highboy_nfc_iso_dep_t iso_dep;
    highboy_nfc_iso14443b_t iso14443b;
    highboy_nfc_iso15693_t iso15693;
  } data;
} highboy_nfc_card_t;

#ifdef __cplusplus
}
#endif

#endif // HIGHBOY_NFC_TYPES_H
