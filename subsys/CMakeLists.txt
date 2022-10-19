zephyr_library()

zephyr_include_directories(sit)
zephyr_include_directories(sit_ble)
zephyr_include_directories(sit_led)

add_subdirectory_ifdef(CONFIG_SIT sit)
add_subdirectory_ifdef(CONFIG_SIT_BLE sit_ble)
add_subdirectory_ifdef(CONFIG_SIT_LED sit_led)
