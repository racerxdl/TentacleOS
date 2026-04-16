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

// Gallagher — Enterprise access control
// Uses EM4100 customer code 0x09.
// Data layout in lower 32 bits:
// Bits [31:24] = 8-bit region code
// Bits [23:8]  = 16-bit cardholder number
// Bits [7:4]   = 4-bit issue level
// Bits [3:0]   = 4-bit checksum nibble
// Ref: https://github.com/megabug/gallagher-research

#include "rfid_protocol_decoder.h"

#include "esp_log.h"

static const char *TAG = "PROTO_GALLAGHER";

#define GALLAGHER_BITS        40
#define GALLAGHER_CUSTOMER_ID 0x09
#define GALLAGHER_REGION_MASK 0xFF
#define GALLAGHER_CARD_MASK   0xFFFF
#define GALLAGHER_ISSUE_MASK  0x0F

static bool protocol_gallagher_decode(const ys_rfid2_raw_data_t *raw,
                                      rfid_decoded_data_t *out_data) {
  if (raw->bit_count != 40) {
    return false;
  }

  if (raw->data[0] != GALLAGHER_CUSTOMER_ID) {
    return false;
  }

  uint8_t region = raw->data[1];
  uint16_t card_number = ((uint16_t)raw->data[2] << 8) | raw->data[3];
  uint8_t issue_level = (raw->data[4] >> 4) & GALLAGHER_ISSUE_MASK;
  uint8_t checksum = raw->data[4] & 0x0F;

  // Simple checksum: XOR of all nibbles should produce a known value
  uint8_t calc = 0;
  for (int i = 0; i < 4; i++) {
    calc ^= (raw->data[i] >> 4) ^ (raw->data[i] & 0x0F);
  }
  calc ^= (raw->data[4] >> 4);

  if ((calc & 0x0F) != checksum) {
    return false;
  }

  uint64_t full_value = 0;
  for (int i = 0; i < YS_RFID2_RAW_DATA_LEN; i++) {
    full_value = (full_value << 8) | raw->data[i];
  }

  out_data->protocol_name = "Gallagher";
  out_data->facility_code = region;
  out_data->card_number = card_number;
  out_data->bit_count = GALLAGHER_BITS;
  out_data->raw_value = full_value;

  ESP_LOGD(TAG, "Region: %u, Card: %u, Issue: %u", region, card_number, issue_level);
  return true;
}

rfid_protocol_t protocol_gallagher = {
    .name = "Gallagher",
    .decode = protocol_gallagher_decode,
};
