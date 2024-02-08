/**********************************************************************************
 * 
 *  Copyright (C) 2023  Sven Hoyer
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
***********************************************************************************/

/** 
 * @file main.c
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Main File for start and running the SIT System.
 *
 * The main of Sports Indoor Tracking System. This File Handles the subsys. 
 * Here main handle will start. 
 *  
 * @bug No known bugs.
 * @todo everything 
 */
#include "deca_device_api.h"

#include <sit/sit.h>
#include <sit/sit_device.h>
#include <sit/sit_config.h>
#include <sit_led/sit_led.h>

#include <sit_ble/ble_init.h>
#include <sit_ble/ble_device.h>

#include <stdio.h>
#include <string.h>

#define APP_NAME "Sports Indoor Tracking\n"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sit_main, LOG_LEVEL_INF);

void initialization() {
	uint8_t error = 0;

	sit_led_init();

	if (sit_ble_init()) {
		LOG_ERR("Bluetooth init failed");
	}
	// repeat configuration when failed
	do {
		error = sit_init();
	} while (error > 1);
	
	LOG_INF("Init Fertig ");
}

int main(int argc, char *argv[])  {
	printk(APP_NAME);
	printk("==================\n");
	init_device_id();

	initialization();
	sit_run_forever();
	return 0;
}
