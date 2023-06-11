# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_SIT sit.c)
zephyr_library_sources_ifdef(CONFIG_SIT sit_config.c)
zephyr_library_sources_ifdef(CONFIG_SIT sit_device.c)
zephyr_library_sources_ifdef(CONFIG_SIT_DIAGNOSTIC sit_diagnostic.c)
zephyr_library_sources_ifdef(CONFIG_SIT sit_distance.c)
zephyr_library_sources_ifdef(CONFIG_SIT sit_utils.c)

target_sources(app PRIVATE ../../drivers/platform/port.c ../../drivers/platform/config_options.c)


zephyr_include_directories(../../drivers/platform)