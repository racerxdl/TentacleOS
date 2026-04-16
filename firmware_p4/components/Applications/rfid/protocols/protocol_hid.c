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

// HID Prox H10301 — 26-bit Wiegand format
// Bit 25:    even parity (covers bits 25-13)
// Bits 24-17: 8-bit facility code (0-255)
// Bits 16-1:  16-bit card number (0-65535)
// Bit 0:     odd parity (covers bits 12-0)
// Ref: https://www.proxcards.com/hid-26-bit-h10301-format/

#include "rfid_protocol_decoder.h"

#include "esp_log.h"

static const char *TAG = "PROTO_HID";

#define HID_WIEGAND_BITS    26
#define HID_FC_SHIFT        17
#define HID_FC_MASK         0xFF
#define HID_CARD_SHIFT      1
#define HID_CARD_MASK       0xFFFF
#define HID_EVEN_PARITY_BIT 25
#define HID_ODD_PARITY_BIT  0

static int count_bits(uint32_t value, int from_bit, int to_bit) {
  int count = 0;
  for (int i = from_bit; i <= to_bit; i++) {
    if (value & (1u << i)) {
      count++;
    }
  }
  return count;
}

static bool protocol_hid_decode(const ys_rfid2_raw_data_t *raw, rfid_decoded_data_t *out_data) {
  if (raw->bit_count != 40) {
    return false;
  }

  // Use lower 26 bits of the 40-bit raw value
  uint64_t full_value = 0;
  for (int i = 0; i < YS_RFID2_RAW_DATA_LEN; i++) {
    full_value = (full_value << 8) | raw->data[i];
  }

  uint32_t wiegand = (uint32_t)(full_value & 0x03FFFFFF);

  // Extract fields
  uint8_t even_parity = (wiegand >> HID_EVEN_PARITY_BIT) & 1;
  uint8_t facility_code = (wiegand >> HID_FC_SHIFT) & HID_FC_MASK;
  uint16_t card_number = (wiegand >> HID_CARD_SHIFT) & HID_CARD_MASK;
  uint8_t odd_parity = (wiegand >> HID_ODD_PARITY_BIT) & 1;

  // Validate even parity (bits 25-13, including parity bit itself)
  int even_count = count_bits(wiegand, 13, HID_EVEN_PARITY_BIT);
  if ((even_count % 2) != 0) {
    return false;
  }

  // Validate odd parity (bits 12-0, including parity bit itself)
  int odd_count = count_bits(wiegand, HID_ODD_PARITY_BIT, 12);
  if ((odd_count % 2) != 1) {
    return false;
  }

  out_data->protocol_name = "HID Prox";
  out_data->facility_code = facility_code;
  out_data->card_number = card_number;
  out_data->bit_count = HID_WIEGAND_BITS;
  out_data->raw_value = wiegand;

  ESP_LOGD(TAG, "FC: %u, Card: %u", facility_code, card_number);
  return true;
}

rfid_protocol_t protocol_hid = {
    .name = "HID Prox",
    .decode = protocol_hid_decode,
};
