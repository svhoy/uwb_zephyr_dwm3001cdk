# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_SIT_BLE ble_device.c)
zephyr_library_sources_ifdef(CONFIG_SIT_BLE ble_init.c)

zephyr_library_sources_ifdef(CONFIG_CTS cts.c)
