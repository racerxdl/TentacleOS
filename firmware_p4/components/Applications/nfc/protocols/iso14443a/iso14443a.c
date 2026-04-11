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
 * @file iso14443a.c
 * @brief ISO14443A CRC_A calculation.
 * Reference: ISO/IEC 14443-3A.
 */
#include "iso14443a.h"

#define ISO14443A_CRC_INIT 0x6363U
#define ISO14443A_CRC_LEN  2U

void iso14443a_crc(const uint8_t *data, size_t len, uint8_t crc[2]) {
  uint32_t wCrc = ISO14443A_CRC_INIT;
  for (size_t i = 0; i < len; i++) {
    uint8_t bt = data[i];
    bt = (bt ^ (uint8_t)(wCrc & 0x00FF));
    bt = (bt ^ (bt << 4));
    wCrc = (wCrc >> 8) ^ ((uint32_t)bt << 8) ^ ((uint32_t)bt << 3) ^ ((uint32_t)bt >> 4);
  }
  crc[0] = (uint8_t)(wCrc & 0xFF);
  crc[1] = (uint8_t)((wCrc >> 8) & 0xFF);
}

bool iso14443a_check_crc(const uint8_t *data, size_t len) {
  if (len < (ISO14443A_CRC_LEN + 1U))
    return false;
  uint8_t crc[2];
  iso14443a_crc(data, len - ISO14443A_CRC_LEN, crc);
  return (crc[0] == data[len - 2]) && (crc[1] == data[len - 1]);
}
