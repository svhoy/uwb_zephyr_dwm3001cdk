/**
 * Copyright (c) 2015 - Decawave Ltd, Dublin, Ireland.
 * Copyright (c) 2022 - Sven Hoyer
 * 
 * This is an Simple Bluetooth Low Energy Example for the 
 * DWM1001-DEV Board Zephyr
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include "drivers/sit_led/sit_led.h"
#include "drivers/sit_ble/ble_device.h"

#define APP_NAME "SIMPLE BLE EXAMPLE\n"

void main(void) {
	printk(APP_NAME);
	printk("==================\n");

	dwm_led_init();
	ble_device_init();

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

		if (is_connected()) {
			/* Battery level simulation */
			bas_notify();

			dwm_set_led(1, 0);
		} else {
			/* Toggle LED while disconnect */
			dwm_toggle_led(1);
		}
		
	}
}
