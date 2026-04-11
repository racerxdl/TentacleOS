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

#include "ir_protocol_jvc.h"

#include "esp_log.h"

#include "ir_protocol.h"

static const char *TAG = "IR_JVC";

bool ir_protocol_jvc_decode(const rmt_symbol_word_t *symbols, size_t count, ir_data_t *out_data) {
  if (symbols == NULL || count == 0 || out_data == NULL)
    return false;

  bool has_header = false;
  size_t data_offset = 0;

  if (count < JVC_MIN_SYMBOLS_REPEAT)
    return false;

  if (count >= JVC_MIN_SYMBOLS_FULL && ir_match(symbols[0].duration0, JVC_HEADER_MARK) &&
      ir_match(symbols[0].duration1, JVC_HEADER_SPACE)) {
    has_header = true;
    data_offset = 1;
  } else if (count >= JVC_MIN_SYMBOLS_REPEAT && count <= JVC_MAX_SYMBOLS_REPEAT &&
             ir_match(symbols[0].duration0, JVC_BIT_MARK)) {
    for (size_t i = 0; i < JVC_REPEAT_VALIDATE_COUNT && i < count; i++) {
      if (!ir_match(symbols[i].duration0, JVC_BIT_MARK))
        return false;
    }
    data_offset = 0;
  } else {
    return false;
  }

  if (count < data_offset + JVC_FRAME_BITS + 1)
    return false;

  ir_pulse_distance_cfg_t cfg = {
      .one_space = JVC_ONE_SPACE,
      .zero_space = JVC_ZERO_SPACE,
      .msb_first = false,
  };
  uint32_t raw = (uint32_t)ir_decode_pulse_distance(symbols, data_offset, JVC_FRAME_BITS, &cfg);

  out_data->protocol = IR_PROTO_JVC;
  out_data->address = raw & JVC_ADDR_MASK;
  out_data->command = (raw >> JVC_CMD_SHIFT) & 0xFF;
  out_data->repeat = !has_header;
  return true;
}

size_t ir_protocol_jvc_encode(const ir_data_t *data, rmt_symbol_word_t *symbols, size_t max) {
  if (data == NULL || symbols == NULL || max == 0)
    return 0;

  uint16_t raw =
      (data->address & JVC_ADDR_MASK) | ((uint16_t)(data->command & 0xFF) << JVC_CMD_SHIFT);

  ir_encode_distance_cfg_t cfg = {
      .header_mark = data->repeat ? 0 : JVC_HEADER_MARK,
      .header_space = data->repeat ? 0 : JVC_HEADER_SPACE,
      .bit_mark = JVC_BIT_MARK,
      .one_space = JVC_ONE_SPACE,
      .zero_space = JVC_ZERO_SPACE,
      .max = max,
      .msb_first = false,
      .stop_bit = true,
  };
  return ir_encode_pulse_distance(symbols, raw, JVC_FRAME_BITS, &cfg);
}