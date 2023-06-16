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
 * @file sit_ble_init.h
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Defniton of functions for BLE init.
 *
 * Init the BLE System for SIT  
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */
#ifndef __BLE_INIT_H__
#define __BLE_INIT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>

#include <sit/sit_config.h>

#include "ble_device.h"
#include "ble_uuids.h" 

/***************************************************************************
* Initilization for BLE  
*
* @return error code 
* [
*	0 -> init ok,
*	err -> init failed
* ]
****************************************************************************/
uint8_t sit_ble_init(void);


bool is_connected(void);
void ble_sit_notify(json_distance_msg_t* json_data, size_t data_len);
int ble_get_command(void);
void bas_notify(void);

int  ble_start_advertising(void);
int  ble_stop_advertising(void);
int  ble_disconnect(void);


#ifdef __cplusplus
}
#endif

#endif  // __BLE_INIT_H__