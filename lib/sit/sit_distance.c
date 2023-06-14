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
 * @file sit_distance.c
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Implementation of functions for distance measurement.
 *
 * This file implement functions for distance measurement for the 
 * DWM3001cdk in the SIT system. 
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */
#include "sit/sit_distance.h"
#include "sit/sit_config.h"
#include "sit/sit_device.h"
#ifdef CONFIG_SIT_DIAGNOSTIC
	#include "sit/sit_diagnostic.h"
#endif


#include <deca_device_api.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SIT_DISTANCE, LOG_LEVEL_INF);

uint32_t status_reg;

diagnostic_info diagnostic; 

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
uint32_t sit_msg_receive() {
	uint32_t l_status_reg;
	waitforsysstatus(&l_status_reg, NULL, (DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR), 0);
	return l_status_reg;
}

void sit_start_poll(uint8_t* msg_data, uint16_t msg_size){
	dwt_writesysstatuslo(DWT_INT_TXFRS_BIT_MASK);
	dwt_writetxdata(msg_size, msg_data, 0); // 0 offset
	dwt_writetxfctrl(msg_size, 0, 1); // frame_length, bufferOffset, ranging bit (0 no ranging, 1 ranging)
	dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);//switch to rx after `setrxaftertxdelay`
}

bool sit_send_at(uint8_t* msg_data, uint16_t size, uint64_t tx_time){
	dwt_setdelayedtrxtime(tx_time);
	dwt_writetxdata(size, msg_data, 0); 
	dwt_writetxfctrl(size, 0, 1); 
	uint8_t late = dwt_starttx(DWT_START_TX_DELAYED);
	if(late == DWT_SUCCESS) {
		waitforsysstatus(&status_reg, NULL, DWT_INT_TXFRS_BIT_MASK, 0);
		dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK); // write to clear send status bit
		LOG_DBG("Send Success");
		return true;
	} else {
		recover_tx_errors();
		status_reg = dwt_readsysstatuslo();
		LOG_WRN("sit_sendAt() - dwt_starttx() late");
		return false;
	}
}

void sit_receive_at(uint16_t timeout) {
	dwt_setrxtimeout(timeout); // 0 : disable timeout
	dwt_rxenable(DWT_START_RX_IMMEDIATE); //DWT_START_RX_DELAYED only used with dwt_setdelayedtrxtime() before 
}

bool sit_check_msg(uint8_t* data, uint16_t expected_size) {
	bool result = false;
	status_reg = sit_msg_receive();
	LOG_INF("Test: %08x & %08x", status_reg, DWT_INT_RXFCG_BIT_MASK);
	if(status_reg & DWT_INT_RXFCG_BIT_MASK) {
		/* Clear good RX frame event in the DW IC status register. */
		dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK);
		uint16_t size = dwt_getframelength();
		if (size == expected_size) {
			dwt_readrxdata(data, size, 0);
			#ifdef CONFIG_SIT_DIAGNOSTIC
				get_diagnostic(&diagnostic);
			#endif
			result = true;
		} else {
			LOG_ERR("rx size %u != expected_size %u",size,expected_size);
		}
	} else {
		dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		LOG_WRN("sit_checkReceivedMessage() no 'RX Frame Checksum Good'");
		uint32_t reg2 = dwt_readsysstatuslo();
		uint32_t reg_removed = (status_reg ^ reg2);
		LOG_WRN("sit_checkReceivedMessage() reg1 = 0x%08x ; reg2 = 0x%08x ; removed =  0x%08x",status_reg,reg2,reg_removed);
		status_reg = reg2;
	}

	return result;
}

bool sit_check_msg_id(msg_id_t id, msg_header_t* message) {
	
	bool result = false;
	if(sit_check_msg((uint8_t*)message, sizeof(msg_header_t))){
		if(message->header.id == id) {
			result = true;
		} else {
			LOG_ERR("sit_checkReceivedId() mismatch id(%u/%u)",(uint8_t)id,(uint8_t)message->header.id);
		}
	} else {
		LOG_ERR("sit_checkReceivedId(%d, %d) fail",(uint8_t)id, (uint8_t)message->header.id);
	}
	return result;
}

bool sit_check_final_msg_id(msg_id_t id, msg_ss_twr_final_t* message) {
	bool result = false;
	if(sit_check_msg((uint8_t*)message, sizeof(msg_ss_twr_final_t))){
		if(message->header.id == id) {
			result = true;
		} else {
			LOG_ERR("sit_checkReceivedIdFinalMsg() mismatch id(%u/%u)",(uint8_t)id,(uint8_t)message->header.id);
		}
	} else {
		LOG_ERR("sit_checkReceivedIdFinalMsg(%u,header) fail",(uint8_t)id);
	}
	return result;
}

void sit_set_rx_tx_delay_rx_timeout(uint32_t delay_us, uint16_t timeout) {
	dwt_setrxaftertxdelay(delay_us);
	dwt_setrxtimeout(timeout);
}

void recover_tx_errors() {
	uint32_t status = dwt_readsysstatuslo();
	if(status & DWT_INT_RXFCE_BIT_MASK) {
		LOG_INF("recovering TX errors 0x%08x", (status & DWT_INT_RXFCE_BIT_MASK));
		dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
	}
}