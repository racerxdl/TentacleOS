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

#ifndef MESHTASTIC_CRYPTO_PKI_H
#define MESHTASTIC_CRYPTO_PKI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MT_PKI_OVERHEAD 12  /* 8-byte AES-CCM tag + 4-byte extra nonce */

/**
 * @brief Encrypt a payload using X25519 + AES-256-CCM (Meshtastic PKI).
 *
 * Derives shared_key = SHA256(X25519(our_priv, peer_pub)), uses AES-256-CCM
 * with 13-byte nonce built from (packet_id, from_node, random extra_nonce).
 * Output layout: [ciphertext(plen)] [auth_tag(8)] [extra_nonce(4)].
 *
 * @param peer_pub   32-byte Curve25519 public key of destination
 * @param from_node  Sender node number (included in nonce)
 * @param packet_id  Packet id (included in nonce)
 * @param plain      Plaintext input
 * @param plen       Plaintext length
 * @param out        Output buffer (must be at least plen + MT_PKI_OVERHEAD)
 * @return true on success, false on failure (weak key, mbedtls error)
 */
bool mt_pki_encrypt_curve25519(const uint8_t *peer_pub,
                                uint32_t from_node,
                                uint32_t packet_id,
                                const uint8_t *plain,
                                size_t plen,
                                uint8_t *out);

/**
 * @brief Decrypt a payload produced by mt_pki_encrypt_curve25519.
 *
 * Reads extra_nonce from cipher[clen-4..clen-1] and auth_tag from
 * cipher[clen-12..clen-5], derives shared_key, and AEAD-decrypts.
 *
 * @param peer_pub   32-byte Curve25519 public key of sender
 * @param from_node  Sender node number
 * @param packet_id  Packet id
 * @param cipher     Ciphertext input (including trailing tag + extra_nonce)
 * @param clen       Total length including MT_PKI_OVERHEAD
 * @param out        Output buffer (must be at least clen - MT_PKI_OVERHEAD)
 * @return true on success (auth tag valid), false on failure
 */
bool mt_pki_decrypt_curve25519(const uint8_t *peer_pub,
                                uint32_t from_node,
                                uint32_t packet_id,
                                const uint8_t *cipher,
                                size_t clen,
                                uint8_t *out);

#ifdef __cplusplus
}
#endif

#endif // MESHTASTIC_CRYPTO_PKI_H
