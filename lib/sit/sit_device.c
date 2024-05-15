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
 * @file sit_device.c
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief SIT Device functions.
 *
 * Device Functions Definiton to handle the device status (e.g.read status register)
 *
 * @bug No known bugs.
 * @todo everything 
 */
#include "sit/sit_device.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <deca_device_api.h>

#include <zephyr/drivers/hwinfo.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sit_device, LOG_LEVEL_INF);

char m_deviceID[17];

uint32_t sit_get_device_status(void) {
	return dwt_readsysstatuslo();
}

void waitforsysstatus(uint32_t *lo_result, uint32_t *hi_result, uint32_t lo_mask, uint32_t hi_mask)
{
    uint32_t lo_result_tmp = 0;
    uint32_t hi_result_tmp = 0;

    // If a mask has been passed into the function for the system status register (lower 32-bits)
    if (lo_mask)
    {
        while (!((lo_result_tmp = dwt_readsysstatuslo()) & (lo_mask)))
        {
            // If a mask value is set for the system status register (higher 32-bits)
            if (hi_mask)
            {
                // If mask value for the system status register (higher 32-bits) is found
                if ((hi_result_tmp = dwt_readsysstatushi()) & hi_mask)
                {
                    break;
                }
            }
        }
    }
    // if only a mask value for the system status register (higher 32-bits) is set
    else if (hi_mask)
    {
        while (!((hi_result_tmp = dwt_readsysstatushi()) & (hi_mask))) { };
    }

    if (lo_result != NULL)
    {
        *lo_result = lo_result_tmp;
    }

    if (hi_result != NULL)
    {
        *hi_result = hi_result_tmp;
    }
}

void set_antenna_delay(
    uint16_t rx_delay,
    uint16_t tx_delay
) {
    dwt_setrxantennadelay(rx_delay);
	dwt_settxantennadelay(tx_delay);
}

uint16_t get_tx_ant_dly() {
    return dwt_gettxantennadelay();
}

uint16_t get_rx_ant_dly() {
    return dwt_getrxantennadelay();
}

// Device ID
void init_device_id(void) {
	uint8_t buf_deviceID[8];
	hwinfo_get_device_id(buf_deviceID, sizeof(buf_deviceID));

	for(int i = 0; i < sizeof(buf_deviceID); i++){
		sprintf(m_deviceID + 2*i, "%02X", buf_deviceID[i]);
	}
    m_deviceID[16] = '\0';
}

void get_device_id(char **deviceID) {
    *deviceID = strdup(m_deviceID);
}