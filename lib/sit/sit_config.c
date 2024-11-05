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

#include <deca_device_api.h>

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
    .measurement_type = ds_3_twr,
    .diagnostic = false,
    .min_measurement = 0,
    .max_measurement = 0,
};

dwt_config_t sit_device_config = {
    9,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    DWT_SFD_DW_8,     /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

void set_device_state(char *command) {
    if (strcmp(command, "start") == 0) {
		device_settings.state = measurement;
	} else if (strcmp(command, "stop") == 0) {
        device_settings.state = sleep;
        device_type = none;
        dwt_forcetrxoff();
    } else {
        LOG_ERR("Wrong command");
    }
}

void set_device_id(uint8_t device_id) {
    device_settings.deviceID = device_id;
    LOG_INF("Device ID: %d", device_settings.deviceID);
}

void set_device_type(char *type) {
    if (strcmp(type, "initiator") == 0) {
        device_type = initiator;
    } else if (strcmp(type, "responder") == 0) {
        device_type = responder;
    } else if (strcmp(type, "A") == 0) {
        device_type = dev_a;
    } else if (strcmp(type, "B") == 0) {
        device_type = dev_b;
    } else if (strcmp(type, "C") == 0) {
        device_type = dev_c;
    } else {
        LOG_ERR("Wrong device type");
    }
}

void set_responder(uint8_t responder) {
    device_settings.responder = responder;
}

void set_min_measurement(uint32_t measurement) {
    device_settings.min_measurement = measurement;
}

void set_max_measurement(uint32_t measurement) {
    device_settings.max_measurement = measurement;
}

void set_measurement_type(char *measurement_type) {
    LOG_INF("Measurement type: %s", measurement_type);
    if (strcmp(measurement_type, "ss_twr") == 0) {
        device_settings.measurement_type = ss_twr;
    } else if (strcmp(measurement_type, "ds_3_twr") == 0) {
        device_settings.measurement_type = ds_3_twr;
    } else if (strcmp(measurement_type, "two_device") == 0) {
        device_settings.measurement_type = two_device_calibration;
    }
    else {
        LOG_ERR("Wrong measurement type");
    }
    
}

void set_rx_ant_dly(uint16_t dly) {
    device_settings.rx_ant_dly = dly;
    dwt_setrxantennadelay(dly);
}
void set_tx_ant_dly(uint16_t dly) {
    device_settings.tx_ant_dly = dly;
    dwt_settxantennadelay(dly);
}