# Copyright (c) 2025 HIGH CODE LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

file(GLOB_RECURSE SD_CARD_SERVICE_SRCS "sd_card/*.c")
file(GLOB_RECURSE CONSOLE_SERVICE_SRCS "console/*.c")
file(GLOB_RECURSE CONSOLE_COMMANDS_SRCS "console/commands/*.c")
file(GLOB_RECURSE C5_FLASHER_SRCS "c5_flasher/*.c")
file(GLOB_RECURSE SPI_BRIDGE_SRCS "spi_bridge/*.c")
file(GLOB_RECURSE BRIDGE_MANAGER_SRCS "bridge_manager/*.c")
file(GLOB_RECURSE BT_SERVICE_SRCS "bluetooth/*.c")
file(GLOB_RECURSE STORAGE_API_SRCS "storage_api/*.c")
file(GLOB_RECURSE STORAGE_VFS_SRCS "storage_vfs/*.c")
file(GLOB_RECURSE STORAGE_ASSETS_SRCS "storage_assets/*.c")

idf_component_register(SRCS
  "font/font.c"
  "icons/icons.c"
  "wifi/wifi_service.c"

  ${STORAGE_API_SRCS}
  ${STORAGE_VFS_SRCS}
  ${STORAGE_ASSETS_SRCS}

  "ir/ir.c"
  "ir/ir_file.c"
  "ir/ir_protocol.c"
  "ir/ir_protocol_nec.c"
  "ir/ir_protocol_samsung.c"
  "ir/ir_protocol_lg.c"
  "ir/ir_protocol_jvc.c"
  "ir/ir_protocol_denon.c"
  "ir/ir_protocol_panasonic.c"
  "ir/ir_protocol_rc6.c"
  "ir/ir_protocol_rc5.c"
  "ir/ir_protocol_sony.c"
  "lvgl_port/lv_port_indev.c"
  "lvgl_port/lv_port_disp.c"
  ${SD_CARD_SERVICE_SRCS}
  ${CONSOLE_SERVICE_SRCS}
  ${CONSOLE_COMMANDS_SRCS}
  ${C5_FLASHER_SRCS}
  ${SPI_BRIDGE_SRCS}
  ${BRIDGE_MANAGER_SRCS}
  ${BT_SERVICE_SRCS}

  "ota/ota_service.c"

  INCLUDE_DIRS
  "font/include"
  "icons/include"
  "wifi/include"
  "ir/include"
  "storage_api/include"
  "storage_vfs/include"
  "storage_assets/include"
  "lvgl_port/include"
  "sd_card/include"
  "console/include"
  "spi_bridge/include"
  "c5_flasher/include"
  "bridge_manager/include"
  "bluetooth/include"
  "ota/include"




  REQUIRES 
    sdmmc
    driver  
    vfs
    fatfs
    esp_timer
    esp_common
    Drivers
    Applications
    esp_http_server
    nvs_flash
    lvgl
    littlefs
    cjson
    console 
    argtable3
    app_update
)

# Embed C5 Firmware Binary into Service component (used by c5_flasher)
set(C5_BIN_PATH "${CMAKE_SOURCE_DIR}/../firmware_c5/build/TentacleOS_C5.bin")
set(C5_FIRMWARE_EMBEDDED 0)
if(EXISTS ${C5_BIN_PATH})
    target_add_binary_data(${COMPONENT_LIB} "${C5_BIN_PATH}" BINARY)
    set(C5_FIRMWARE_EMBEDDED 1)
    message(STATUS "Embedding C5 Firmware: ${C5_BIN_PATH}")
else()
    message(WARNING "C5 Firmware binary not found at ${C5_BIN_PATH}. Firmware update feature will be disabled.")
endif()

target_compile_definitions(${COMPONENT_LIB} PRIVATE C5_FIRMWARE_EMBEDDED=${C5_FIRMWARE_EMBEDDED})
