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

#ifndef ASSETS_MANAGER_H
#define ASSETS_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "lvgl.h"

/** @brief Initialize the assets manager and load assets from flash. */
void assets_manager_init(void);

/** @brief Get a cached asset image descriptor by path. */
lv_image_dsc_t *assets_get(const char *path);

/** @brief Free all loaded assets and release memory. */
void assets_manager_free_all(void);

/** @brief Load assets from SD card directory, overriding flash assets. */
int assets_load_from_sd(const char *sd_dir, const char *flash_prefix);

/** @brief Unload all SD-sourced assets from the cache. */
void assets_unload_sd(void);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_MANAGER_H