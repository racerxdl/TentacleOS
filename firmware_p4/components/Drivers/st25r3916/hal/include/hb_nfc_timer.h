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

#ifndef HB_NFC_TIMER_H
#define HB_NFC_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Busy-wait delay in microseconds.
 *
 * Uses the ESP-ROM implementation. Safe for ISRs, but blocks the CPU.
 *
 * @param us Delay in microseconds.
 */
void hb_nfc_timer_delay_us(uint32_t us);

/**
 * @brief OS-aware delay in milliseconds.
 *
 * Yields the current task to the FreeRTOS scheduler.
 * Must NOT be called from an ISR.
 *
 * @param ms Delay in milliseconds. If 0, performs a task yield.
 */
void hb_nfc_timer_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif // HB_NFC_TIMER_H
