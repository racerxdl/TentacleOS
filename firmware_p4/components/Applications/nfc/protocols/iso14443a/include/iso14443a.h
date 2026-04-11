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
 * @file iso14443a.h
 * @brief ISO14443A types and CRC_A.
 */
#ifndef ISO14443A_H
#define ISO14443A_H

#include <stdint.h>
#include <stddef.h>
#include "highboy_nfc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NFC_LOG_HEX_BUF_SIZE 128

/**
 * @brief Calculate CRC_A with initial value 0x6363.
 *
 * @param data  Input data buffer.
 * @param len   Length of data in bytes.
 * @param[out] crc  Output buffer for 2-byte CRC result.
 */
void iso14443a_crc(const uint8_t *data, size_t len, uint8_t crc[2]);

/**
 * @brief Verify CRC_A on received data.
 *
 * @param data  Input data with CRC in the last 2 bytes.
 * @param len   Total length including CRC bytes.
 * @return
 *   - true if CRC is valid
 *   - false otherwise
 */
bool iso14443a_check_crc(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* ISO14443A_H */
