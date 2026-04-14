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

#ifndef YS_RFID2_PARSER_H
#define YS_RFID2_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "ys_rfid2_types.h"

/**
 * @brief Reset the parser state, discarding any accumulated bytes.
 */
void ys_rfid2_parser_reset(void);

/**
 * @brief Feed a byte into the parser.
 *
 * Accumulates bytes until the '@' delimiter is found, then attempts to
 * extract a valid card ID from the "card number: xxxxxxxxxx@" frame.
 *
 * @param      byte     Byte received from UART.
 * @param[out] out_raw  Filled with card data if a valid frame was parsed.
 * @return true if a valid card ID was extracted, false otherwise.
 */
bool ys_rfid2_parser_feed(uint8_t byte, ys_rfid2_raw_data_t *out_raw);

#ifdef __cplusplus
}
#endif

#endif // YS_RFID2_PARSER_H
