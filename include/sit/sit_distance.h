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
 * @file sit_distance.h
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Defniton of functions for distance measurement.
 *
 * This file defines functions for distance measurement for the 
 * DWM3001cdk in the SIT system. 
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */

#ifndef __SIT_DISTANCE_H__
#define __SIT_DISTANCE_H__

#include "sit_config.h"

#include <stdint.h>
#include <stdbool.h>

extern diagnostic_info diagnostic; 

/***************************************************************************
 * Start ranging with a poll msg 
 *
 * @param uint8_t* msg_data ->  pointer to the data you like to send with
 *                              the poll msg
 * @param uint16_t msg_size ->  length of the data you like to send 
 *
 * @return None
 *
****************************************************************************/
void sit_start_poll(uint8_t* msg_data, uint16_t msg_size);

/***************************************************************************
 * Send a 
 *
 * @param uint8_t* msg_data ->  pointer to the data you like to send with
 *                              the poll msg
 * @param uint16_t msg_size ->  length of the data you like to send 
 * @param unit64_t tx_time  ->  start sending msg at this system time
 *
 * @return bool true  -> if  the msg is send at the tx_time
 *         bool false -> if the msg is send to late 
 *
****************************************************************************/
bool sit_send_at(uint8_t* msg_data, uint16_t size, uint64_t tx_time);

/***************************************************************************
 * Start ranging with a poll msg 
 *
 * @param uint8_t* msg_data ->  pointer to the data you like to send with
 *                              the poll msg
 * @param uint16_t msg_size ->  length of the data you like to send 
 *
 * @return None
 *
****************************************************************************/
void sit_receive_at(uint16_t timeout);

bool sit_check_msg_id(msg_id_t id, msg_header_t* message);

bool sit_check_final_msg_id(msg_id_t id, msg_ss_twr_final_t* message);

void sit_set_rx_tx_delay_rx_timeout(uint32_t delay_us,uint16_t timeout);

void recover_tx_errors();

#endif