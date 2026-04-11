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

#ifndef UI_BLE_SPAM_H
#define UI_BLE_SPAM_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Open the BLE spam screen. */
void ui_ble_spam_open(void);

/** @brief Set the displayed spam attack name. */
void ui_ble_spam_set_name(const char *name);

#ifdef __cplusplus
}
#endif

#endif // UI_BLE_SPAM_H
