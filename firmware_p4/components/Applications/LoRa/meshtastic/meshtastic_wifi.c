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

#include "meshtastic_wifi.h"

#include "esp_log.h"

#include "meshtastic_phone_bridge.h"

static const char *TAG = "MESH_WIFI";

esp_err_t meshtastic_wifi_init(uint32_t node_num) {
  (void)node_num;
  esp_err_t ret = meshtastic_phone_bridge_wifi_start();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "WiFi start failed: %s", esp_err_to_name(ret));
  }
  return ret;
}

void meshtastic_wifi_poll(void) {}

bool meshtastic_wifi_is_connected(void) {
  return meshtastic_phone_bridge_is_connected();
}

int meshtastic_wifi_get_client_fd(void) {
  return -1;
}
