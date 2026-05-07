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

#include "beacon_spam.h"

#include <string.h>

#include "esp_log.h"

#include "spi_bridge.h"
#include "spi_session.h"

static const char *TAG = "BEACON_SPAM";

static bool s_is_running = false;
static uint32_t s_session_id = SPI_SESSION_INVALID_ID;

bool beacon_spam_start_custom(const char *json_path) {
  if (json_path == NULL)
    return false;
  size_t len = strnlen(json_path, SPI_MAX_PAYLOAD - 1);
  s_session_id = spi_session_start(
      SPI_ID_WIFI_APP_BEACON_SPAM, (uint8_t *)json_path, (uint8_t)len, NULL, NULL);
  s_is_running = (s_session_id != SPI_SESSION_INVALID_ID);
  if (!s_is_running)
    ESP_LOGW(TAG, "Failed to start beacon spam (custom list) over SPI");
  return s_is_running;
}

bool beacon_spam_start_random(void) {
  s_session_id = spi_session_start(SPI_ID_WIFI_APP_BEACON_SPAM, NULL, 0, NULL, NULL);
  s_is_running = (s_session_id != SPI_SESSION_INVALID_ID);
  if (!s_is_running)
    ESP_LOGW(TAG, "Failed to start beacon spam (random) over SPI");
  return s_is_running;
}

void beacon_spam_stop(void) {
  if (s_session_id != SPI_SESSION_INVALID_ID) {
    spi_session_stop(s_session_id);
    s_session_id = SPI_SESSION_INVALID_ID;
  }
  s_is_running = false;
}

bool beacon_spam_is_running(void) {
  return s_is_running;
}
