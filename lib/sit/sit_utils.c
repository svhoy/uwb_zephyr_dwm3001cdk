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
 * @file sit_utils.c
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Utils Functions for SIT System.
 *
 * This contains function implemantation for handle the DWM 3001 functionality that 
 * doesn't have to do directly with distance measurement. 
 *
 * @bug No known bugs.
 */

#include "sit/sit_utils.h"

#include <deca_device_api.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SIT_Utils, LOG_LEVEL_INF);


uint64_t get_tx_timestamp_u64(void)
{
	uint8_t ts_tab[5];
	uint64_t ts = 0;
	int i;

	dwt_readtxtimestamp(ts_tab);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

uint64_t get_rx_timestamp_u64(void)
{
	uint8_t ts_tab[5];
	uint64_t ts = 0;
	int i;

	dwt_readrxtimestamp(ts_tab);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

