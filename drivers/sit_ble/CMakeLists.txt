# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_TEST ble_device.c)
zephyr_library_sources_ifdef(CONFIG_TEST ble_init.c)

zephyr_library_sources_ifdef(CONFIG_CTS cts.c)
