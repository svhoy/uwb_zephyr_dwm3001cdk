/**
 *
 * Copyright (c) 2022 - Sven Hoyer
 * 
 * This file is part of Zephyr-DWM1001.
 *
 *   Zephyr-DWM1001 is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Zephyr-DWM1001 is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Zephyr-DWM1001.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */
#include <sit/sit.h>
#include <sit_led/sit_led.h>
#include <sit_json/sit_json.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define APP_NAME "Sports Indoor Tracking - JSON\n"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sit_main, LOG_LEVEL_INF);

#define STACK_SIZE 2048

// void main(void) {
// 	printk(APP_NAME);
// 	printk("==================\n");

// 	sit_led_init();
// 	// Problem! in this version of zephyr there is no float in json possible
// 	//char json_msg[] = "{\"type\":\"distance_msg\",\"data\":{\"state\":\"start\",\"distance\":1,\"test_id\":44}}";
// 	char json_msg[] = "{\"type\":\"measurement_msg\",\"command\":\"start\"}";

// 	int expected_return_code = (1 << ARRAY_SIZE(command_msg_descr)) - 1;
// 	json_command_msg_t distance_msg;

// 	// int ret = json_distance_parser(
// 	// 			json_msg,
// 	// 			sizeof(json_msg),
// 	// 			&distance_msg
// 	// );
// 	int ret = json_command_parser(
// 				json_msg,
// 				sizeof(json_msg),
// 				&distance_msg
// 	);

// 	if (ret < 0) {
// 		LOG_ERR("JSON Parse Error: %d", ret);
// 	}
// 	else if (ret != expected_return_code) {
// 		LOG_ERR("Not all values decoded; Expected return code %d but got %d", expected_return_code, ret);
// 	}
// 	else {
// 		LOG_INF("json_obj_parse return code: %d", ret);
// 		LOG_INF("calculated return code: %d", expected_return_code);
// 		// if (strcmp(distance_msg.data.state, "start"))
// 		// {
// 		// 	LOG_INF("State: start");
// 		// }
// 		// else
// 		// {
// 		// 	LOG_INF("State: False");
// 		// }
// 		// LOG_INF("Distance: %d", distance_msg.data.distance);
// 		// LOG_INF("Test ID: %d", distance_msg.data.test_id);
// 		if(strcmp(distance_msg.command, "start")) {
// 			LOG_INF("Test");
// 		}
// 		LOG_INF("Distance: %s", distance_msg.type);
// 		LOG_INF("Test ID: %s", distance_msg.command);
// 	}

// }

int main(void) {
	printk(APP_NAME);
	printk("==================\n");
	double distance = 3.1415;

	while(42){
		if (distance >= 0) {
			char *json_data;
			LOG_INF("Test");
			uint8_t result = json_encode_distance(&json_data, &distance);
			if(result == 0){
				LOG_INF("BLE MSG: %s", json_data);
				LOG_INF("BLE LEN: %d", strlen(json_data));
			}
			LOG_INF("BLE LEN: %d", result);
			k_free(json_data);
		}
		k_msleep(100);
		LOG_INF("Test Ende");
	}
	return 0;
}

K_THREAD_DEFINE(josn_example, STACK_SIZE,
            main, NULL, NULL, NULL,
            K_HIGHEST_APPLICATION_THREAD_PRIO, 0, 0);