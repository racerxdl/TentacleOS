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

// EM4100 / EM-Marin / TK4100 — 125kHz read-only RFID
// 40-bit data: 8-bit customer/version code + 32-bit unique card ID
// Ref: https://www.priority1design.com.au/em4100_protocol.html

#include "rfid_protocol_decoder.h"

#include "esp_log.h"

static const char *TAG = "PROTO_EM4100";

#define EM4100_CUSTOMER_SHIFT 32
#define EM4100_CUSTOMER_MASK  0xFF
#define EM4100_CARD_MASK      0xFFFFFFFF

static bool protocol_em4100_decode(const ys_rfid2_raw_data_t *raw, rfid_decoded_data_t *out_data) {
  if (raw->bit_count != 40) {
    return false;
  }

  // Reconstruct 40-bit value from raw bytes (big-endian)
  uint64_t value = 0;
  for (int i = 0; i < YS_RFID2_RAW_DATA_LEN; i++) {
    value = (value << 8) | raw->data[i];
  }

  uint8_t customer_code = (uint8_t)((value >> EM4100_CUSTOMER_SHIFT) & EM4100_CUSTOMER_MASK);
  uint32_t card_id = (uint32_t)(value & EM4100_CARD_MASK);

  out_data->protocol_name = "EM4100";
  out_data->facility_code = customer_code;
  out_data->card_number = card_id;
  out_data->bit_count = 40;
  out_data->raw_value = value;

  ESP_LOGD(TAG, "Customer: %u, Card: %lu", customer_code, (unsigned long)card_id);
  return true;
}

rfid_protocol_t protocol_em4100 = {
    .name = "EM4100",
    .decode = protocol_em4100_decode,
};
