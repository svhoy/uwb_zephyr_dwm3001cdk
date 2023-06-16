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
 * @file sit_config.c
 * @author Sven Hoyer (svhoy)
 * @date 14.05.2023
 * @brief Defniton config options for SIT system.
 *
 * This file defines different configurations for the SIT system. 
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */

#include "sit/sit_config.h"

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SIT_CONFIG, LOG_LEVEL_INF);


device_settings_t device_settings = {
    .deviceID = "A",
    .state = sleep,
    .diagnostic = false,
};

void set_device_state(char *command) {
    if (strcmp(command, "start") == 0) {
		device_settings.state = measurement;
	} else if (strcmp(command, "stop") == 0) {
        device_settings.state = sleep;
    } else {
        LOG_ERR("Wrong command");
    }
}