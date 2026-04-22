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

#include "mt_mod_text.h"

#include <string.h>

#include "esp_log.h"

static const char *TAG = "MT_MOD_TEXT";

#define MT_TEXT_DEDUP_RING 50
#define MT_TEXT_MAX_LEN    220

static uint32_t s_seen_ids[MT_TEXT_DEDUP_RING];
static uint8_t s_seen_idx = 0;
static uint32_t s_node_num = 0;

static bool has_seen_recently(uint32_t id) {
  for (int i = 0; i < MT_TEXT_DEDUP_RING; i++) {
    if (s_seen_ids[i] == id)
      return true;
  }
  return false;
}

static void mark_seen(uint32_t id) {
  s_seen_ids[s_seen_idx] = id;
  s_seen_idx = (s_seen_idx + 1) % MT_TEXT_DEDUP_RING;
}

void mt_mod_text_init(uint32_t node_num) {
  s_node_num = node_num;
  memset(s_seen_ids, 0, sizeof(s_seen_ids));
  s_seen_idx = 0;
  ESP_LOGI(TAG, "Initialized — dedup ring=%d", MT_TEXT_DEDUP_RING);
}

void mt_mod_text_on_received(const mt_packet_meta_t *meta,
                             const uint8_t *payload,
                             uint16_t len,
                             bool is_compressed) {
  if (has_seen_recently(meta->id)) {
    ESP_LOGD(TAG, "Duplicate text id=0x%08lX, dropping", (unsigned long)meta->id);
    return;
  }
  mark_seen(meta->id);

  char text[MT_TEXT_MAX_LEN];
  uint16_t copy_len = len;
  if (copy_len >= sizeof(text))
    copy_len = sizeof(text) - 1;
  memcpy(text, payload, copy_len);
  text[copy_len] = 0;

  if (is_compressed) {
    ESP_LOGI(TAG, "Compressed text from=0x%08lX (%u bytes)", (unsigned long)meta->from, copy_len);
  } else {
    ESP_LOGI(TAG,
             "Text from=0x%08lX ch=%u RSSI=%d SNR=%d: \"%s\"",
             (unsigned long)meta->from,
             meta->channel,
             meta->rssi_dbm,
             meta->snr_db,
             text);
  }
}
