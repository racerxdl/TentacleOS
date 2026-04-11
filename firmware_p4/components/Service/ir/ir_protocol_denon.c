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

#include "ir_protocol_denon.h"

#include "esp_log.h"

#include "ir_protocol.h"

static const char *TAG = "IR_DENON";

bool ir_protocol_denon_decode(const rmt_symbol_word_t *symbols, size_t count, ir_data_t *out_data) {
  if (symbols == NULL || count == 0 || out_data == NULL)
    return false;

  if (count < DENON_MIN_SYMBOLS)
    return false;
  if (symbols[0].duration0 > DENON_MARK_MAX_US)
    return false;

  for (size_t i = 0; i < DENON_VALIDATE_MARK_COUNT; i++) {
    if (!ir_match(symbols[i].duration0, DENON_BIT_MARK)) {
      ESP_LOGD(TAG, "Mark validation failed at symbol %d", i);
      return false;
    }
  }

  ir_pulse_distance_cfg_t cfg = {
      .one_space = DENON_ONE_SPACE,
      .zero_space = DENON_ZERO_SPACE,
      .msb_first = false,
  };
  uint32_t raw = (uint32_t)ir_decode_pulse_distance(symbols, 0, DENON_FRAME_BITS, &cfg);

  out_data->protocol = IR_PROTO_DENON;
  out_data->address = raw & DENON_ADDR_MASK;
  out_data->command = (raw >> DENON_CMD_SHIFT) & 0xFF;
  out_data->repeat = false;
  return true;
}

size_t ir_protocol_denon_encode(const ir_data_t *data, rmt_symbol_word_t *symbols, size_t max) {
  if (data == NULL || symbols == NULL || max == 0)
    return 0;

  uint16_t raw =
      (data->address & DENON_ADDR_MASK) | ((uint16_t)(data->command & 0xFF) << DENON_CMD_SHIFT);

  ir_encode_distance_cfg_t cfg = {
      .header_mark = 0,
      .header_space = 0,
      .bit_mark = DENON_BIT_MARK,
      .one_space = DENON_ONE_SPACE,
      .zero_space = DENON_ZERO_SPACE,
      .max = max,
      .msb_first = false,
      .stop_bit = true,
  };
  return ir_encode_pulse_distance(symbols, raw, DENON_FRAME_BITS, &cfg);
}