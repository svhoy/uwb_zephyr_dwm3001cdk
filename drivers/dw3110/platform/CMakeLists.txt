# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_DW3110 deca_mutex.c)
zephyr_library_sources_ifdef(CONFIG_DW3110 deca_sleep.c)
zephyr_library_sources_ifdef(CONFIG_DW3110 deca_spi.c)
zephyr_library_sources_ifdef(CONFIG_DW3110 port.c)

