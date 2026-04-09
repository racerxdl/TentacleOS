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

#ifndef KERNEL_H
#define KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all kernel subsystems and start background tasks.
 */
void kernel_init(void);

/**
 * @brief Log a safeguard alert.
 *
 * @param title    Alert title.
 * @param message  Alert message.
 */
void safeguard_alert(const char *title, const char *message);

#ifdef __cplusplus
}
#endif

#endif // KERNEL_H
