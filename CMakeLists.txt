# Point to root directory.
set(HSM_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "root directory")

add_subdirectory(samples)
add_subdirectory(drivers)

add_subdirectory_ifdef(CONFIG_DW3000 subsys)
add_subdirectory_ifdef(CONFIG_DW3000 dw3000-decadriver/dwt_uwb_driver)
add_subdirectory_ifdef(CONFIG_DW3000 dw3000-decadriver/platform)
add_subdirectory_ifdef(CONFIG_DW3000 platform)
