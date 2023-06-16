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
 * @file sit_ble_device.h
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Defniton of device functions for ble.
 *
 * Defines functions to set and get BLE device informations e.g. name, address. 
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */
#ifndef __BLE_DEVICE_H__
#define __BLE_DEVICE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "ble_init.h"

uint8_t ble_set_device_name(const char *name);
void ble_device_name(void);
void ble_device_address(void);
void ble_device_init(void);

#ifdef __cplusplus
}
#endif


#endif //__BLE_DEVICE_H__