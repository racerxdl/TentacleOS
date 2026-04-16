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

// Indala 26-bit (Motorola/Identiv format 40134)
// 26 bits with proprietary bit scrambling:
// 1 even parity + 12-bit site code + 12-bit card number + 1 odd parity
// The bit positions are scrambled within the raw data.
// Ref: Identiv 4000/4020 clamshell card specs

#include "rfid_protocol_decoder.h"

#include "esp_log.h"

static const char *TAG = "PROTO_INDALA";

#define INDALA_BITS      26
#define INDALA_SITE_MASK 0xFFF
#define INDALA_CARD_MASK 0xFFF

static bool protocol_indala_decode(const ys_rfid2_raw_data_t *raw, rfid_decoded_data_t *out_data) {
  if (raw->bit_count != 40) {
    return false;
  }

  uint64_t full_value = 0;
  for (int i = 0; i < YS_RFID2_RAW_DATA_LEN; i++) {
    full_value = (full_value << 8) | raw->data[i];
  }

  // Use lower 26 bits
  uint32_t indala = (uint32_t)(full_value & 0x03FFFFFF);

  // Extract parity bits
  uint8_t even_parity = (indala >> 25) & 1;
  uint8_t odd_parity = indala & 1;

  // Extract site code (bits 24-13) and card number (bits 12-1)
  uint16_t site_code = (uint16_t)((indala >> 13) & INDALA_SITE_MASK);
  uint16_t card_number = (uint16_t)((indala >> 1) & INDALA_CARD_MASK);

  // Validate even parity (covers upper half: bits 25-13)
  int even_count = 0;
  for (int i = 13; i <= 25; i++) {
    if (indala & (1u << i)) {
      even_count++;
    }
  }
  if ((even_count % 2) != 0) {
    return false;
  }

  // Validate odd parity (covers lower half: bits 12-0)
  int odd_count = 0;
  for (int i = 0; i <= 12; i++) {
    if (indala & (1u << i)) {
      odd_count++;
    }
  }
  if ((odd_count % 2) != 1) {
    return false;
  }

  out_data->protocol_name = "Indala";
  out_data->facility_code = site_code;
  out_data->card_number = card_number;
  out_data->bit_count = INDALA_BITS;
  out_data->raw_value = indala;

  ESP_LOGD(TAG, "Site: %u, Card: %u", site_code, card_number);
  return true;
}

rfid_protocol_t protocol_indala = {
    .name = "Indala",
    .decode = protocol_indala_decode,
};
