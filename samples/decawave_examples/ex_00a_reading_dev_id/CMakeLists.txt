cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
set(DTS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
set(BOARD qorvo_dwm3001cdk)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ex_00a_reading_dev_id)

target_sources(app PRIVATE src/main.c)

target_include_directories(app PRIVATE ../../../include/drivers/dw3110)
