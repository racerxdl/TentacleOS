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

#include "ir_protocol_samsung.h"

#include "esp_log.h"

#include "ir_protocol.h"

static const char *TAG = "IR_SAMSUNG";

bool ir_protocol_samsung_decode(const rmt_symbol_word_t *symbols,
                                size_t count,
                                ir_data_t *out_data) {
  if (symbols == NULL || count == 0 || out_data == NULL)
    return false;
  if (count < SAMSUNG_MIN_SYMBOLS)
    return false;

  if (!ir_match(symbols[0].duration0, SAMSUNG_HEADER_MARK) ||
      !ir_match(symbols[0].duration1, SAMSUNG_HEADER_SPACE))
    return false;

  ir_pulse_distance_cfg_t cfg = {
      .one_space = SAMSUNG_ONE_SPACE,
      .zero_space = SAMSUNG_ZERO_SPACE,
      .msb_first = false,
  };
  uint32_t raw = (uint32_t)ir_decode_pulse_distance(symbols, 1, SAMSUNG_FRAME_BITS, &cfg);

  uint8_t addr_lo = (raw >> 0) & SAMSUNG_ADDR_STANDARD_MAX;
  uint8_t addr_hi = (raw >> SAMSUNG_ADDR_HI_SHIFT) & SAMSUNG_ADDR_STANDARD_MAX;
  uint8_t cmd = (raw >> SAMSUNG_CMD_SHIFT) & SAMSUNG_ADDR_STANDARD_MAX;
  uint8_t cmd_inv = (raw >> SAMSUNG_CMD_INV_SHIFT) & SAMSUNG_ADDR_STANDARD_MAX;

  if ((uint8_t)(cmd ^ cmd_inv) != SAMSUNG_INTEGRITY_MASK) {
    ESP_LOGD(TAG, "CMD integrity check failed");
    return false;
  }

  out_data->protocol = IR_PROTO_SAMSUNG;
  out_data->command = cmd;
  out_data->repeat = false;
  out_data->address = (addr_lo == addr_hi) ? addr_lo : (raw & SAMSUNG_EXT_ADDR_MASK);
  return true;
}

size_t ir_protocol_samsung_encode(const ir_data_t *data, rmt_symbol_word_t *symbols, size_t max) {
  if (data == NULL || symbols == NULL || max == 0)
    return 0;

  uint8_t cmd = data->command & SAMSUNG_ADDR_STANDARD_MAX;
  uint32_t raw;
  if (data->address <= SAMSUNG_ADDR_STANDARD_MAX) {
    uint8_t addr = data->address & SAMSUNG_ADDR_STANDARD_MAX;
    raw = addr | ((uint32_t)addr << SAMSUNG_ADDR_HI_SHIFT) | ((uint32_t)cmd << SAMSUNG_CMD_SHIFT) |
          ((uint32_t)(~cmd & SAMSUNG_INTEGRITY_MASK) << SAMSUNG_CMD_INV_SHIFT);
  } else {
    raw = data->address | ((uint32_t)cmd << SAMSUNG_CMD_SHIFT) |
          ((uint32_t)(~cmd & SAMSUNG_INTEGRITY_MASK) << SAMSUNG_CMD_INV_SHIFT);
  }

  ir_encode_distance_cfg_t cfg = {
      .header_mark = SAMSUNG_HEADER_MARK,
      .header_space = SAMSUNG_HEADER_SPACE,
      .bit_mark = SAMSUNG_BIT_MARK,
      .one_space = SAMSUNG_ONE_SPACE,
      .zero_space = SAMSUNG_ZERO_SPACE,
      .max = max,
      .msb_first = false,
      .stop_bit = true,
  };
  return ir_encode_pulse_distance(symbols, raw, SAMSUNG_FRAME_BITS, &cfg);
}