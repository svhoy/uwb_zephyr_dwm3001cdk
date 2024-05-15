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
 * @file sit.h
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief SIT System entry header.
 *
 * This is the main entry in the sit system it contains functions for 
 * init, start stop measurements, define type of measurements and config 
 * the system.
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>


#include <zephyr/kernel.h>
#include <deca_device_api.h>

#define SPEED_OF_LIGHT 299702547
extern uint32_t measurements;
/***************************************************************************
* Initilization for DW3001 -> SPI Connection, DW3000, Antenna Delay  
*
* @return error code 
* [
*	-1 -> dwt_initialise failed,
*	-2 -> dwt_configure failed,
*   -3 -> dwt_probe failed
* ]
****************************************************************************/
uint8_t sit_init();
void sit_run_forever();

void sit_sstwr_initiator();
void sit_sstwr_responder();

void sit_dstwr_initiator(); 
void sit_dstwr_responder();

void reset_sequence();
