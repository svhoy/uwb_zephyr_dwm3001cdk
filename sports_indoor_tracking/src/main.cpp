/**
 *
 * Copyright (c) 2022 - Sven Hoyer
 * 
 * Sports Indoor Tracking System Main
 * 
 */
#include <sit.h>
#include <sit_led.h>
#include <sit_json.h>

#include <ble_init.h>
#include <ble_device.h>

#define APP_NAME "SIMPLE JSON EXAMPLE\n"

#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(sit_main, LOG_LEVEL_INF);


void main(void) {
	printk(APP_NAME);
	printk("==================\n");

	sit_led_init();
	ble_init();
	
	while(42) { //Life, the universe, and everything

	}

}
