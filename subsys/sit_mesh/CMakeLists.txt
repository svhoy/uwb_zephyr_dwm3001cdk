# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_SIT sit.cpp)

target_sources(app PRIVATE ../../platform/port.c ../../platform/config_options.c)


zephyr_include_directories(../../platform)