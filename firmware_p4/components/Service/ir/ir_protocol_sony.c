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

#include "ir_protocol_sony.h"

#include "esp_log.h"

#include "ir_protocol.h"

static const char *TAG = "IR_SONY";

bool ir_protocol_sony_decode(const rmt_symbol_word_t *symbols, size_t count, ir_data_t *out_data) {
  if (symbols == NULL || count == 0 || out_data == NULL)
    return false;
  if (!ir_match(symbols[0].duration1, SONY_HEADER_SPACE)) {
    ESP_LOGD(TAG, "Header space mismatch");
    return false;
  }

  size_t data_bits;
  if (count >= SONY_SIRC20_MIN_SYMBOLS)
    data_bits = SONY_SIRC20_BITS;
  else if (count >= SONY_SIRC15_MIN_SYMBOLS)
    data_bits = SONY_SIRC15_BITS;
  else if (count >= SONY_SIRC12_MIN_SYMBOLS)
    data_bits = SONY_SIRC12_BITS;
  else
    return false;

  ir_pulse_width_cfg_t cfg = {
      .one_mark = SONY_ONE_MARK,
      .zero_mark = SONY_ZERO_MARK,
      .msb_first = false,
  };
  uint64_t raw = ir_decode_pulse_width(symbols, 1, data_bits, &cfg);

  out_data->protocol = IR_PROTO_SONY;
  out_data->command = raw & SONY_CMD_MASK;
  out_data->address = (raw >> SONY_ADDR_SHIFT) & SONY_ADDR_MASK;
  out_data->repeat = false;
  return true;
}

size_t ir_protocol_sony_encode(const ir_data_t *data, rmt_symbol_word_t *symbols, size_t max) {
  if (data == NULL || symbols == NULL || max == 0)
    return 0;

  size_t num_bits;
  if (data->address > SONY_SIRC15_ADDR_MAX)
    num_bits = SONY_SIRC20_BITS;
  else if (data->address > SONY_SIRC12_ADDR_MAX)
    num_bits = SONY_SIRC15_BITS;
  else
    num_bits = SONY_SIRC12_BITS;

  uint64_t raw = (data->command & SONY_CMD_MASK) | ((uint64_t)(data->address) << SONY_ADDR_SHIFT);

  ir_encode_width_cfg_t cfg = {
      .header_mark = SONY_HEADER_MARK,
      .header_space = SONY_HEADER_SPACE,
      .one_mark = SONY_ONE_MARK,
      .zero_mark = SONY_ZERO_MARK,
      .bit_space = SONY_BIT_SPACE,
      .max = max,
      .msb_first = false,
      .stop_bit = false,
  };
  return ir_encode_pulse_width(symbols, raw, num_bits, &cfg);
}
