# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(DTS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(BOARD qorvo_dwm3001cdk)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sit_ble_json)

# the binary only library from Qorvo has problems with -Os or -O2
zephyr_library_compile_options(-O1)

target_sources(app PRIVATE
  src/main.c
)

target_include_directories(app PRIVATE ../../drivers/platform)


