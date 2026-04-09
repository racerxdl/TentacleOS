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

#include "uuid_lookup.h"

#include <stdio.h>

static const char *TAG = "UUID_LOOKUP";

typedef struct {
  uint16_t uuid16;
  const char *name;
} uuid_lookup_entry_t;

static const uuid_lookup_entry_t UUID_TABLE[] = {
    {0x1800, "Generic Access"},
    {0x1801, "Generic Attribute"},
    {0x180A, "Device Information"},
    {0x180D, "Heart Rate"},
    {0x180F, "Battery Service"},
    {0x1812, "Human Interface Device"},
    {0x181A, "Environmental Sensing"},
    {0x2A00, "Device Name"},
    {0x2A01, "Appearance"},
    {0x2A19, "Battery Level"},
    {0x2A29, "Manufacturer Name"},
    {0x2A37, "Heart Rate Measurement"},
    {0x2A4D, "Report"},
};
#define UUID_TABLE_COUNT (sizeof(UUID_TABLE) / sizeof(UUID_TABLE[0]))

const char *uuid_get_name(const ble_uuid_t *uuid) {
  if (uuid->type == BLE_UUID_TYPE_16) {
    uint16_t u16 = BLE_UUID16(uuid)->value;
    for (size_t i = 0; i < UUID_TABLE_COUNT; i++) {
      if (UUID_TABLE[i].uuid16 == u16)
        return UUID_TABLE[i].name;
    }
  }
  return "Unknown Service/Char";
}
