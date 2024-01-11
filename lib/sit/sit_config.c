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

device_type_t device_type = none;

device_settings_t device_settings = {
    .deviceID = 0,
    .initiator = 1,
    .responder = 0,
    .tx_ant_dly = 16385,
    .rx_ant_dly = 16385,
    .state = sleep,
    .measurement_type = ss_twr,
    .diagnostic = false,
    .min_measurement = 0,
    .max_measurement = 0,
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

void set_device_id(uint8_t device_id) {
    device_settings.deviceID = device_id;
}

void set_responder(uint8_t responder) {
    device_settings.responder = responder;
}

void set_min_measurement(uint8_t measurement) {
    device_settings.min_measurement = measurement;
}

void set_max_measurement(uint8_t measurement) {
    device_settings.max_measurement = measurement;
}

void set_measurement_type(char *measurement_state) {
    if (strcmp(measurement_state, "ss_twr") == 0) {
        device_settings.measurement_type = ss_twr;
    } else if (strcmp(measurement_state, "ds_3_twr") == 0) {
        device_settings.measurement_type = ds_3_twr;
    }
    
}

void set_rx_ant_dly(uint16_t dly) {
    device_settings.rx_ant_dly = dly;
}
void set_tx_ant_dly(uint16_t dly) {
    device_settings.tx_ant_dly = dly;
}