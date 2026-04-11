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
 * @file t2t_emu.h
 * @brief NFC-A Type 2 Tag (T2T) Emulation ST25R3916.
 */
#ifndef T2T_EMU_H
#define T2T_EMU_H

#include <stdint.h>
#include <stdbool.h>
#include "highboy_nfc_error.h"
#include "highboy_nfc_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Supported T2T tag types for emulation.
 */
typedef enum {
  T2T_TAG_NTAG213 = 0,  /**< NTAG213 (144 bytes user memory). */
  T2T_TAG_NTAG215,      /**< NTAG215 (504 bytes user memory). */
  T2T_TAG_NTAG216,      /**< NTAG216 (888 bytes user memory). */
  T2T_TAG_ULTRALIGHT_C, /**< MIFARE Ultralight C. */
} t2t_tag_type_t;

/**
 * @brief T2T emulation configuration.
 */
typedef struct {
  t2t_tag_type_t type;   /**< Tag type to emulate. */
  const char *ndef_text; /**< NDEF text record payload (NULL for empty). */

  uint8_t uid[7];  /**< Tag UID (4 or 7 bytes). */
  uint8_t uid_len; /**< Actual UID length. */

  bool enable_pwd; /**< Enable NTAG password authentication. */
  uint8_t pwd[4];  /**< NTAG password bytes. */
  uint8_t pack[2]; /**< NTAG password acknowledge bytes. */
  uint8_t auth0;   /**< First page requiring authentication. */
  uint8_t access;  /**< Access configuration byte. */

  bool enable_ulc_auth; /**< Enable Ultralight C 3DES authentication. */
  uint8_t ulc_key[16];  /**< Ultralight C 3DES key (16 bytes). */
} t2t_emu_config_t;

/**
 * @brief Initialise T2T emulator with a custom configuration.
 *
 * @param cfg  Emulation configuration (copied internally).
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_INVALID_ARG if cfg is NULL
 */
hb_nfc_err_t t2t_emu_init(const t2t_emu_config_t *cfg);

/**
 * @brief Initialise T2T emulator with default NTAG213 + sample NDEF.
 *
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_IO on communication failure
 */
hb_nfc_err_t t2t_emu_init_default(void);

/**
 * @brief Configure ST25R3916 as NFC-A target for T2T emulation.
 *
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_IO on communication failure
 */
hb_nfc_err_t t2t_emu_configure_target(void);

/**
 * @brief Start emulation (enter SLEEP state, field detector on).
 *
 * @return
 *   - HB_NFC_OK on success
 *   - HB_NFC_ERR_IO on communication failure
 */
hb_nfc_err_t t2t_emu_start(void);

/**
 * @brief Stop emulation and power down.
 */
void t2t_emu_stop(void);

/**
 * @brief Return pointer to internal tag memory (for live inspection).
 *
 * @return Pointer to the emulated tag memory array.
 */
uint8_t *t2t_emu_get_mem(void);

/**
 * @brief Run one emulation step (call in tight loop). Non-blocking.
 */
void t2t_emu_run_step(void);

#ifdef __cplusplus
}
#endif

#endif /* T2T_EMU_H */
