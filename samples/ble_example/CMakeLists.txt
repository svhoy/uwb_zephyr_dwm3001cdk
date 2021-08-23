cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ble_example)

target_sources(app PRIVATE src/main.c)

target_sources(app PRIVATE ../../ble/ble_device.c)
target_sources(app PRIVATE ../../ble/ble_base.c)
target_sources(app PRIVATE ../../ble/ble_service.c)

target_include_directories(app PRIVATE ../../ble/)


zephyr_library_include_directories($ENV{ZEPHYR_BASE}/samples/bluetooth)
zephyr_library_include_directories($ENV{ZEPHYR_BASE}/samples/bluetooth/gatt/)
