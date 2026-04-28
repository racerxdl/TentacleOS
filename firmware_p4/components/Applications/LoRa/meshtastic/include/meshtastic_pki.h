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

#ifndef MESHTASTIC_PKI_H
#define MESHTASTIC_PKI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "esp_err.h"

#define MT_PKI_KEY_LEN 32

/**
 * @brief Initialize PKI keypair (X25519 Curve25519).
 *
 * Loads the persisted keypair from NVS. If absent, generates a new
 * X25519 keypair, persists privkey and pubkey in NVS, and uses the
 * freshly generated material for the current session. Must run once
 * at boot after mt_nvs_init.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL if keygen fails (RNG or mbedtls internal)
 */
esp_err_t mt_pki_init(void);

/**
 * @brief Pointer to our 32-byte X25519 public key (little-endian).
 *
 * Valid for the lifetime of the program. Intended for inclusion in
 * NodeInfo/User.public_key (field 8).
 */
const uint8_t *mt_pki_get_pubkey(void);

/**
 * @brief Pointer to our 32-byte X25519 private key (little-endian).
 *
 * Reserved for future ECDH during DM end-to-end encryption. Must not
 * leave the device.
 */
const uint8_t *mt_pki_get_privkey(void);

#ifdef __cplusplus
}
#endif

#endif // MESHTASTIC_PKI_H
