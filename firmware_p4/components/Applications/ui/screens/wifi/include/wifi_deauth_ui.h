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

#ifndef WIFI_DEAUTH_UI_H
#define WIFI_DEAUTH_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_wifi_types.h"

/** @brief Set the target AP for deauth. */
void ui_wifi_deauth_set_target(wifi_ap_record_t *ap);

/** @brief Open the Wi-Fi deauth screen. */
void ui_wifi_deauth_open(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_DEAUTH_UI_H
