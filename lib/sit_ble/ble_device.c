/* 
 *  ble_device.c 
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>

#include "sit_ble/ble_device.h"
#include "sit_ble/ble_init.h"

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ble_device);

uint8_t ble_set_device_name(const char * name)
{
    int err;

    bt_set_name(name);
    if(err < 0) {
        LOG_ERR("Name not set: %d", err);
    }

    return 0;
}

void ble_device_address(void){
    char addr_s[BT_ADDR_LE_STR_LEN];
	bt_addr_le_t addr = {0};
    size_t count = 1;
    bt_id_get(&addr, &count);
    bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));
    printk("Device address: %s \n", addr_s);
}

void ble_device_name(void) {
    printk("Device Name: %s\n", bt_get_name());
}

void ble_device_init(void) {
    sit_ble_init();
}