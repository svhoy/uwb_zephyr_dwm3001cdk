/* 
 *  ble_device.c 
 */
#include <zephyr/types.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>

#include "drivers/ble/ble_device.h"
#include "drivers/ble/ble_init.h"

#define LOG_LEVEL 3
#include <logging/log.h>
LOG_MODULE_REGISTER(ble_device);

int ble_set_device_name(const char * name)
{
    int err;

    bt_set_name(name);
    if(err < 0) {
        printk("Name not set: %d", err);
    }

    return 0;
}

void ble_device_name(void) {
    printk("Device Name: %s\n", bt_get_name());
}

void ble_device_init(void) {
    ble_init();
}