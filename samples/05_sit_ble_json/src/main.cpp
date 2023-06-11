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
#include <sit.h>
#include <sit_led.h>
#include <sit_json.h>

#define APP_NAME "Sports Indoor Tracking\n"

#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(sit_main, LOG_LEVEL_INF);


void main(void) {
	printk(APP_NAME);
	printk("==================\n");

	sit_led_init();
	// Problem! in this version of zephyr there is no float in json possible
	char json_msg[] = "{\"type\":\"distance_msg\",\"data\":{\"state\":\"start\",\"distance\":1,\"test_id\":44}}";
	
	int expected_return_code = (1 << ARRAY_SIZE(distance_msg_descr)) - 1;
	struct json_distance_msg distance_msg;

	int ret = json_distance_parser(
				json_msg,
				sizeof(json_msg),
				&distance_msg
	);

	if (ret < 0) {
		LOG_ERR("JSON Parse Error: %d", ret);
	}
	else if (ret != expected_return_code) {
		LOG_ERR("Not all values decoded; Expected return code %d but got %d", expected_return_code, ret);
	}
	else {
		LOG_INF("json_obj_parse return code: %d", ret);
		LOG_INF("calculated return code: %d", expected_return_code);
		if (strcmp(distance_msg.data.state, "start"))
		{
			LOG_INF("State: start");
		}
		else
		{
			LOG_INF("State: False");
		}
		LOG_INF("Distance: %d", distance_msg.data.distance);
		LOG_INF("Test ID: %d", distance_msg.data.test_id);
	}

}
