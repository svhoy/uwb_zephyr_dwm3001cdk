# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_SIT_BLE ble_device.cpp)
zephyr_library_sources_ifdef(CONFIG_SIT_BLE ble_init.cpp)

zephyr_library_sources_ifdef(CONFIG_CTS cts.cpp)
