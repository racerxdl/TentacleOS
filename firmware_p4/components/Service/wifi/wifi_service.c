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

#include "wifi_service.h"

#include <string.h>

#include "esp_log.h"

#include "spi_bridge.h"

static const char *TAG = "WIFI_SERVICE_P4";

#define SSID_MAX_LEN           33
#define SPI_TIMEOUT_MS         2000
#define SPI_SCAN_TIMEOUT_MS    20000
#define SPI_CONNECT_TIMEOUT_MS 15000
#define SPI_DATA_TIMEOUT_MS    1000

static wifi_ap_record_t s_cached_record;
static char s_connected_ssid[SSID_MAX_LEN] = {0};

void wifi_service_init(void) {
  spi_bridge_send_command(SPI_ID_WIFI_START, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
}

void wifi_service_deinit(void) {
  spi_bridge_send_command(SPI_ID_WIFI_STOP, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
}

void wifi_service_start(void) {
  spi_bridge_send_command(SPI_ID_WIFI_START, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
}

void wifi_service_stop(void) {
  spi_bridge_send_command(SPI_ID_WIFI_STOP, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
}

void wifi_service_scan(void) {
  spi_bridge_send_command(SPI_ID_WIFI_SCAN, NULL, 0, NULL, NULL, SPI_SCAN_TIMEOUT_MS);
}

uint16_t wifi_service_get_ap_count(void) {
  spi_header_t resp;
  uint8_t payload[2];
  uint16_t magic_count = SPI_DATA_INDEX_COUNT;

  if (spi_bridge_send_command(
          SPI_ID_SYSTEM_DATA, (uint8_t *)&magic_count, 2, &resp, payload, SPI_DATA_TIMEOUT_MS) ==
      ESP_OK) {
    uint16_t count;
    memcpy(&count, payload, 2);
    return count;
  }
  return 0;
}

wifi_ap_record_t *wifi_service_get_ap_record(uint16_t index) {
  spi_header_t resp;

  if (spi_bridge_send_command(SPI_ID_SYSTEM_DATA,
                              (uint8_t *)&index,
                              2,
                              &resp,
                              (uint8_t *)&s_cached_record,
                              SPI_DATA_TIMEOUT_MS) == ESP_OK) {
    return &s_cached_record;
  }
  return NULL;
}

esp_err_t wifi_service_connect_to_ap(const char *ssid, const char *password) {
  spi_wifi_connect_t cfg = {0};
  strncpy(cfg.ssid, ssid, sizeof(cfg.ssid));
  if (password != NULL) {
    strncpy(cfg.password, password, sizeof(cfg.password));
  }
  return spi_bridge_send_command(
      SPI_ID_WIFI_CONNECT, (uint8_t *)&cfg, sizeof(cfg), NULL, NULL, SPI_CONNECT_TIMEOUT_MS);
}

bool wifi_service_is_connected(void) {
  spi_header_t resp;
  spi_system_status_t sys = {0};

  if (spi_bridge_send_command(
          SPI_ID_SYSTEM_STATUS, NULL, 0, &resp, (uint8_t *)&sys, SPI_DATA_TIMEOUT_MS) == ESP_OK) {
    return sys.wifi_connected != 0;
  }
  return (wifi_service_get_connected_ssid() != NULL);
}

const char *wifi_service_get_connected_ssid(void) {
  spi_header_t resp;

  if (spi_bridge_send_command(SPI_ID_WIFI_GET_STA_INFO,
                              NULL,
                              0,
                              &resp,
                              (uint8_t *)s_connected_ssid,
                              SPI_DATA_TIMEOUT_MS) == ESP_OK) {
    s_connected_ssid[SSID_MAX_LEN - 1] = '\0';
    return s_connected_ssid;
  }
  return NULL;
}

bool wifi_service_is_active(void) {
  spi_header_t resp;
  spi_system_status_t sys = {0};

  if (spi_bridge_send_command(
          SPI_ID_SYSTEM_STATUS, NULL, 0, &resp, (uint8_t *)&sys, SPI_DATA_TIMEOUT_MS) == ESP_OK) {
    return sys.wifi_active != 0;
  }
  return false;
}

void wifi_service_change_to_hotspot(const char *new_ssid) {
  wifi_service_set_ap_ssid(new_ssid);
}

esp_err_t wifi_service_save_ap_config(
    const char *ssid, const char *password, uint8_t max_conn, const char *ip_addr, bool enabled) {
  spi_wifi_ap_config_t cfg = {0};

  if (ssid == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  strncpy(cfg.ssid, ssid, sizeof(cfg.ssid));
  if (password != NULL) {
    strncpy(cfg.password, password, sizeof(cfg.password));
  }
  cfg.max_conn = max_conn;
  if (ip_addr != NULL) {
    strncpy(cfg.ip_addr, ip_addr, sizeof(cfg.ip_addr));
  }
  cfg.enabled = enabled ? 1 : 0;

  return spi_bridge_send_command(
      SPI_ID_WIFI_SAVE_AP_CONFIG, (uint8_t *)&cfg, sizeof(cfg), NULL, NULL, SPI_TIMEOUT_MS);
}

esp_err_t wifi_service_set_enabled(bool enabled) {
  uint8_t payload = enabled ? 1 : 0;
  return spi_bridge_send_command(SPI_ID_WIFI_SET_ENABLED, &payload, 1, NULL, NULL, SPI_TIMEOUT_MS);
}

esp_err_t wifi_service_set_ap_ssid(const char *ssid) {
  return spi_bridge_send_command(
      SPI_ID_WIFI_SET_AP, (uint8_t *)ssid, strlen(ssid), NULL, NULL, SPI_TIMEOUT_MS);
}

esp_err_t wifi_service_set_ap_password(const char *password) {
  if (password == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  return spi_bridge_send_command(SPI_ID_WIFI_SET_AP_PASSWORD,
                                 (uint8_t *)password,
                                 strlen(password),
                                 NULL,
                                 NULL,
                                 SPI_TIMEOUT_MS);
}

esp_err_t wifi_service_set_ap_max_conn(uint8_t max_conn) {
  uint8_t payload = max_conn;
  return spi_bridge_send_command(
      SPI_ID_WIFI_SET_AP_MAX_CONN, &payload, 1, NULL, NULL, SPI_TIMEOUT_MS);
}

esp_err_t wifi_service_set_ap_ip(const char *ip_addr) {
  if (ip_addr == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  return spi_bridge_send_command(
      SPI_ID_WIFI_SET_AP_IP, (uint8_t *)ip_addr, strlen(ip_addr), NULL, NULL, SPI_TIMEOUT_MS);
}

void wifi_service_promiscuous_start(wifi_promiscuous_cb_t cb, wifi_promiscuous_filter_t *filter) {
  (void)cb;
  (void)filter;
  esp_err_t err =
      spi_bridge_send_command(SPI_ID_WIFI_PROMISC_START, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
  if (err == ESP_ERR_NOT_SUPPORTED) {
    ESP_LOGW(TAG, "Promiscuous start not supported over SPI");
  }
}

void wifi_service_promiscuous_stop(void) {
  esp_err_t err =
      spi_bridge_send_command(SPI_ID_WIFI_PROMISC_STOP, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
  if (err == ESP_ERR_NOT_SUPPORTED) {
    ESP_LOGW(TAG, "Promiscuous stop not supported over SPI");
  }
}

void wifi_service_start_channel_hopping(void) {
  spi_bridge_send_command(SPI_ID_WIFI_CH_HOP_START, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
}

void wifi_service_stop_channel_hopping(void) {
  spi_bridge_send_command(SPI_ID_WIFI_CH_HOP_STOP, NULL, 0, NULL, NULL, SPI_TIMEOUT_MS);
}
