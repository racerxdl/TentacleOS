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

#ifndef IR_PROTOCOL_RC6_H
#define IR_PROTOCOL_RC6_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir_protocol.h"

/** @brief RC6 biphase unit duration in microseconds. */
#define RC6_UNIT 444

/** @brief RC6 header mark duration in microseconds. */
#define RC6_HEADER_MARK 2666

/** @brief RC6 header space duration in microseconds. */
#define RC6_HEADER_SPACE 889

/** @brief Bit index of the toggle bit within the RC6 frame. */
#define RC6_TOGGLE_INDEX 4

/** @brief Total number of bits in an RC6 Mode 0 frame. */
#define RC6_TOTAL_BITS 21

/** @brief Size of the flat pulse duration buffer used during decode. */
#define RC6_FLAT_BUF_SIZE 128

/** @brief Maximum number of entries consumed from the flat buffer. */
#define RC6_FLAT_BUF_MAX 126

/** @brief Minimum number of successfully decoded bits to accept a frame. */
#define RC6_MIN_DECODED_BITS 17

/** @brief Bit position of the start bit in the raw RC6 frame word. */
#define RC6_START_BIT_POS 20

/** @brief Bit position of the toggle bit in the raw RC6 frame word. */
#define RC6_TOGGLE_BIT_POS 16

/** @brief Bit position of the address field within the RC6 frame word. */
#define RC6_ADDR_SHIFT 8

/** @brief Bit mask for the 8-bit address field in the RC6 frame. */
#define RC6_ADDR_MASK 0xFF

/** @brief Bit mask for the 8-bit command field in the RC6 frame. */
#define RC6_CMD_MASK 0xFF

/**
 * @brief Decode an RC6 Mode 0 IR frame from RMT symbols.
 *
 * Validates the leader pulse, flattens the symbol stream, and decodes
 * biphase-mark encoded bits.
 *
 * @param[in]  symbols   RMT symbol buffer. Must not be NULL.
 * @param[in]  count     Number of symbols. Must be greater than 0.
 * @param[out] out_data  Destination for decoded data. Must not be NULL.
 *
 * @return true if a valid RC6 frame was decoded, false otherwise.
 */
bool ir_protocol_rc6_decode(const rmt_symbol_word_t *symbols, size_t count, ir_data_t *out_data);

/**
 * @brief Encode an RC6 Mode 0 IR command into RMT symbols.
 *
 * Alternates the toggle bit on each call to signal a new keypress.
 *
 * @warning Not thread-safe. Must be called from a single task.
 *
 * @param[in]  data     IR command to encode. Must not be NULL.
 * @param[out] symbols  Destination buffer. Must not be NULL.
 * @param[in]  max      Capacity of @p symbols. Must be at least RC6_FLAT_BUF_SIZE.
 *
 * @return Number of symbols written, or 0 on failure.
 */
size_t ir_protocol_rc6_encode(const ir_data_t *data, rmt_symbol_word_t *symbols, size_t max);

#ifdef __cplusplus
}
#endif

#endif // IR_PROTOCOL_RC6_H