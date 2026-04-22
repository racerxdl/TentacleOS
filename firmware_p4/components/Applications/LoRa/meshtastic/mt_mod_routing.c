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

#include "mt_mod_routing.h"

#include "esp_log.h"

#include "meshtastic_mesh.h"

static const char *TAG = "MT_MOD_ROUTING";

#define MT_ROUTING_HOP_LIMIT 3

static uint32_t s_node_num = 0;

static uint16_t enc_varint(uint8_t *buf, uint64_t value) {
  uint16_t pos = 0;
  do {
    uint8_t byte = value & 0x7F;
    value >>= 7;
    if (value)
      byte |= 0x80;
    buf[pos++] = byte;
  } while (value);
  return pos;
}

static uint16_t enc_field_varint(uint8_t *buf, uint8_t field_num, uint64_t value) {
  buf[0] = (field_num << 3) | 0;
  return 1 + enc_varint(&buf[1], value);
}

void mt_mod_routing_init(uint32_t node_num) {
  s_node_num = node_num;
  ESP_LOGI(TAG, "Initialized");
}

void mt_mod_routing_on_received(const mt_packet_meta_t *meta,
                                const uint8_t *payload,
                                uint16_t len) {
  (void)payload;
  ESP_LOGI(TAG, "Received Routing from=0x%08lX len=%u", (unsigned long)meta->from, len);
}

void mt_mod_routing_maybe_send_ack(const mt_packet_meta_t *meta) {
  uint8_t routing_payload[4];
  uint16_t rlen = 0;
  rlen += enc_field_varint(&routing_payload[rlen], 2, 0);

  ESP_LOGI(TAG,
           "Emitting ACK -> 0x%08lX req_id=0x%08lX",
           (unsigned long)meta->from,
           (unsigned long)meta->id);

  meshtastic_mesh_send_data(meta->from,
                            meta->channel,
                            MT_ROUTING_HOP_LIMIT,
                            MT_PORT_ROUTING,
                            routing_payload,
                            rlen,
                            meta->id,
                            false);
}
