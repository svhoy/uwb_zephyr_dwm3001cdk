# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
set(BOARD decawave_dwm1001_dev)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sit_ss_twr_initiator_ble_example)

target_sources(app PRIVATE  src/main.cpp)
