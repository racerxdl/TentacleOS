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

// Honeywell Nexwatch — 32-bit QuadraKey format
// Uses EM4100 customer code 0x0A.
// Lower 32 bits:
// Bits [31:24] = 8-bit facility code
// Bits [23:0]  = 24-bit card number

#include "rfid_protocol_decoder.h"

#include "esp_log.h"

static const char *TAG = "PROTO_NEXWATCH";

#define NEXWATCH_BITS        32
#define NEXWATCH_CUSTOMER_ID 0x0A
#define NEXWATCH_FC_MASK     0xFF
#define NEXWATCH_CARD_MASK   0x00FFFFFF

static bool protocol_nexwatch_decode(const ys_rfid2_raw_data_t *raw,
                                     rfid_decoded_data_t *out_data) {
  if (raw->bit_count != 40) {
    return false;
  }

  if (raw->data[0] != NEXWATCH_CUSTOMER_ID) {
    return false;
  }

  uint8_t facility_code = raw->data[1];
  uint32_t card_number = ((uint32_t)raw->data[2] << 16) |
                          ((uint32_t)raw->data[3] << 8) |
                          raw->data[4];

  uint64_t full_value = 0;
  for (int i = 0; i < YS_RFID2_RAW_DATA_LEN; i++) {
    full_value = (full_value << 8) | raw->data[i];
  }

  out_data->protocol_name = "Nexwatch";
  out_data->facility_code = facility_code;
  out_data->card_number = card_number;
  out_data->bit_count = NEXWATCH_BITS;
  out_data->raw_value = full_value;

  ESP_LOGD(TAG, "FC: %u, Card: %lu",
      facility_code, (unsigned long)card_number);
  return true;
}

rfid_protocol_t protocol_nexwatch = {
    .name = "Nexwatch",
    .decode = protocol_nexwatch_decode,
};
