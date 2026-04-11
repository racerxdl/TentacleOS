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

#ifndef IR_PROTOCOL_DENON_H
#define IR_PROTOCOL_DENON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ir_protocol.h"

/** @brief Denon bit mark duration in microseconds. */
#define DENON_BIT_MARK 260

/** @brief Denon one-bit space duration in microseconds. */
#define DENON_ONE_SPACE 1820

/** @brief Denon zero-bit space duration in microseconds. */
#define DENON_ZERO_SPACE 780

/** @brief Number of data bits in a Denon frame. */
#define DENON_FRAME_BITS 15

/** @brief Minimum number of RMT symbols for a valid Denon frame. */
#define DENON_MIN_SYMBOLS 16

/** @brief Maximum acceptable mark duration in microseconds for frame detection. */
#define DENON_MARK_MAX_US 600

/** @brief Number of leading marks validated to confirm a Denon frame. */
#define DENON_VALIDATE_MARK_COUNT 3

/** @brief Bit mask for the 5-bit address field in the Denon frame. */
#define DENON_ADDR_MASK 0x1F

/** @brief Bit position of the command field in the Denon frame word. */
#define DENON_CMD_SHIFT 5

/**
 * @brief Decode a Denon IR frame from RMT symbols.
 *
 * Denon uses no header — frame detection relies on validating the leading
 * bit marks against DENON_BIT_MARK.
 *
 * @param[in]  symbols   RMT symbol buffer. Must not be NULL.
 * @param[in]  count     Number of symbols. Must be greater than 0.
 * @param[out] out_data  Destination for decoded data. Must not be NULL.
 *
 * @return true if a valid Denon frame was decoded, false otherwise.
 */
bool ir_protocol_denon_decode(const rmt_symbol_word_t *symbols, size_t count, ir_data_t *out_data);

/**
 * @brief Encode a Denon IR command into RMT symbols.
 *
 * Denon uses no header — the frame starts directly with the first bit mark.
 *
 * @param[in]  data     IR command to encode. Must not be NULL.
 * @param[out] symbols  Destination buffer. Must not be NULL.
 * @param[in]  max      Capacity of @p symbols in symbols.
 *
 * @return Number of symbols written, or 0 on failure.
 */
size_t ir_protocol_denon_encode(const ir_data_t *data, rmt_symbol_word_t *symbols, size_t max);

#ifdef __cplusplus
}
#endif

#endif // IR_PROTOCOL_DENON_H