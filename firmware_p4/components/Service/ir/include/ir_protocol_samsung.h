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

#ifndef IR_PROTOCOL_SAMSUNG_H
#define IR_PROTOCOL_SAMSUNG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir_protocol.h"

/** @brief Samsung header mark duration in microseconds. */
#define SAMSUNG_HEADER_MARK 4500

/** @brief Samsung header space duration in microseconds. */
#define SAMSUNG_HEADER_SPACE 4500

/** @brief Samsung bit mark duration in microseconds. */
#define SAMSUNG_BIT_MARK 560

/** @brief Samsung one-bit space duration in microseconds. */
#define SAMSUNG_ONE_SPACE 1690

/** @brief Samsung zero-bit space duration in microseconds. */
#define SAMSUNG_ZERO_SPACE 560

/** @brief Number of data bits in a Samsung frame. */
#define SAMSUNG_FRAME_BITS 32

/** @brief Minimum number of RMT symbols for a valid Samsung frame. */
#define SAMSUNG_MIN_SYMBOLS 34

/** @brief Bit position of the high address byte in the Samsung frame word. */
#define SAMSUNG_ADDR_HI_SHIFT 8

/** @brief Bit position of the command field in the Samsung frame word. */
#define SAMSUNG_CMD_SHIFT 16

/** @brief Bit position of the inverted command field in the Samsung frame word. */
#define SAMSUNG_CMD_INV_SHIFT 24

/** @brief XOR result expected when a byte and its complement are combined. */
#define SAMSUNG_INTEGRITY_MASK 0xFF

/** @brief Maximum address value for a standard Samsung32 frame (8-bit device). */
#define SAMSUNG_ADDR_STANDARD_MAX 0xFF

/** @brief Bit mask for the 16-bit extended address field in a Samsung32 frame. */
#define SAMSUNG_EXT_ADDR_MASK 0xFFFF

/**
 * @brief Decode a Samsung IR frame from RMT symbols.
 *
 * Validates header, decodes 32 bits, and checks command integrity byte.
 * Supports both standard (8-bit address) and extended (16-bit address) variants.
 *
 * @param[in]  symbols   RMT symbol buffer. Must not be NULL.
 * @param[in]  count     Number of symbols. Must be greater than 0.
 * @param[out] out_data  Destination for decoded data. Must not be NULL.
 *
 * @return true if a valid Samsung frame was decoded, false otherwise.
 */
bool ir_protocol_samsung_decode(const rmt_symbol_word_t *symbols,
                                size_t count,
                                ir_data_t *out_data);

/**
 * @brief Encode a Samsung IR command into RMT symbols.
 *
 * Selects standard (8-bit) or extended (16-bit) address format based on address range.
 * Appends inverted command byte for integrity.
 *
 * @param[in]  data     IR command to encode. Must not be NULL.
 * @param[out] symbols  Destination buffer. Must not be NULL.
 * @param[in]  max      Capacity of @p symbols in symbols.
 *
 * @return Number of symbols written, or 0 on failure.
 */
size_t ir_protocol_samsung_encode(const ir_data_t *data, rmt_symbol_word_t *symbols, size_t max);

#ifdef __cplusplus
}
#endif

#endif // IR_PROTOCOL_SAMSUNG_H