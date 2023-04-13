/**
 * Copyright (c) 2015 - Decawave Ltd, Dublin, Ireland.
 * Copyright (c) 2022 - Sven Hoyer
 * 
 * This is an Simple Bluetooth Low Energy Example for the 
 * DWM1001-DEV Board Zephyr
 */
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

#include "sit_led.h"
#include "ble_device.h"

#define APP_NAME "SIMPLE BLE EXAMPLE\n"

void main(void) {
	printk(APP_NAME);
	printk("==================\n");

	sit_led_init();
	ble_device_init();

	
	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

		if (is_connected()) {
			/* Battery level simulation */
			bas_notify();

			sit_set_led(1, 0);
		} else {
			/* Toggle LED while disconnect */
			sit_toggle_led(1);
		}
		
	}
}
