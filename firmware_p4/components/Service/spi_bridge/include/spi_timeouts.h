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

/**
 * @file spi_timeouts.h
 * @brief Per-command timeouts for the P4–C5 SPI bridge.
 *
 * Used by spi_bridge_get_timeout() to pick the right timeout based
 * on the command ID. Add a new constant + a case in the lookup when
 * a command needs a different timeout than the default.
 */

#ifndef SPI_TIMEOUTS_H
#define SPI_TIMEOUTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_TIMEOUT_DEFAULT_MS 1000
#define SPI_TIMEOUT_WIFI_MS    20000

#ifdef __cplusplus
}
#endif

#endif // SPI_TIMEOUTS_H
