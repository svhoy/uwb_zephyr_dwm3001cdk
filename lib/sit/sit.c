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
 * @file sit.c
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief SIT System entry implemantation.
 *
 * This is the main entry in the sit system it contains functions for 
 * init, start stop measurements, define type of measurements and config 
 * the system.
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */

#include "sit/sit.h"
#include "sit/sit_config.h"
#include "sit/sit_device.h"
#include "sit/sit_distance.h"
#include "sit/sit_utils.h"
#include <sit_led/sit_led.h>

#include <sit_ble/ble_init.h>
#include <sit_ble/ble_device.h>


#include <deca_probe_interface.h>
#include <deca_device_api.h>
#include <port.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SIT_Module, LOG_LEVEL_INF);

extern dwt_config_t sit_device_config;

/**
 * Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. 
 */
dwt_txconfig_t txconfig_options_ch9_sit = {
    0x34,       /* PG delay. */
    0xfefefefe, /* TX power. */
    0x0         /*PG count*/
};

uint32_t sequence = 0;
uint32_t measurements = 0;
double distance = 0.0;
double time_round_1, time_round_2 = 0.0;
double time_reply_1, time_reply_2 = 0.0;
double time_tc_i, time_tc_ii, time_tb_i, time_tb_ii = 0.0;
double time_m21, time_m31, time_a21, time_a31, time_b21, time_b31 = 0.0;


void ble_wait_for_connection() {
	while(!is_connected()) {
		sit_toggle_led(3);
		k_msleep(500);
	}
	sit_set_led(3, 1);
}

void ble_start_connection() {
	ble_start_advertising();
	ble_wait_for_connection();
}

void reset_sequence() {
	sequence = 0;
	measurements = 0;
}

void send_twr_notify(uint8_t responder) {
	if (distance >= 0.0) {
		json_distance_msg_all_t distance_notify = {
			.header = {
				.type = "distance_msg",
				.state = "running",
				.responder = responder,
				.sequence = sequence,
				.measurements = measurements,
			},
			.data = {
				.distance = (float)distance,
				.time_round_1 = (float)time_round_1 * DWT_TIME_UNITS,
				.time_round_2 = (float)time_round_2 * DWT_TIME_UNITS,
				.time_reply_1 = (float)time_reply_1 * DWT_TIME_UNITS,
				.time_reply_2 = (float)time_reply_2 * DWT_TIME_UNITS,
			}, 
			.diagnostic = {
				.rssi_index_resp = diagnostic.rssi,
				.fp_index_resp = diagnostic.fpi,
				.dummy = 0,
				.nlos_percent_resp = diagnostic.nlos,
			}
		};
		ble_sit_notify(&distance_notify, sizeof(distance_notify));
		measurements++;
		if(device_settings.max_measurement != 0 && device_settings.max_measurement <= measurements) {
			device_settings.state = sleep;
		}
	}
}

void send_simple_notify() {
	json_simple_cali_msg_t distance_notify = {
		.header = {
			.type = "cali_msg",
		},
		.data = {
			.time_tc_i = time_tc_i,
			.time_tc_ii = time_tc_ii,
			.time_tb_i = time_tb_i,
			.time_tb_ii = time_tb_ii,
		}
	};
	ble_sit_simple_notify(&distance_notify, sizeof(distance_notify));
	device_settings.state = sleep;
}

void send_two_device_notify() {

}

void sit_sstwr_initiator() {
	while(device_settings.state == measurement) {
		sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
		for(uint8_t responder_id=100; responder_id<=device_settings.responder; responder_id++) {
			msg_simple_t twr_poll = {{twr_1_poll, (uint8_t)sequence, device_settings.deviceID, responder_id}, 0};
			sit_start_poll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));

			msg_ss_twr_final_t rx_final_msg;
			msg_id_t msg_id = ss_twr_2_resp;
			if(sit_check_final_msg_id(msg_id, &rx_final_msg)) {
				uint64_t poll_tx_ts = get_tx_timestamp_u64();
				uint64_t resp_rx_ts = get_rx_timestamp_u64();
				
				uint64_t poll_rx_ts = rx_final_msg.poll_rx_ts;
				uint64_t resp_tx_ts = rx_final_msg.resp_tx_ts;

				double clockOffsetRatio = dwt_readcarrierintegrator() * 
						(FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_9 / 1.0e6) ;

				time_round_1 = (double)resp_rx_ts - poll_tx_ts;
				time_reply_1 = (double)resp_tx_ts - poll_rx_ts;

				double tof =  ((time_round_1 - time_reply_1 * 
						(1 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;
				
				distance = tof * SPEED_OF_LIGHT;

				LOG_INF("TX Power: 0x%08x", (int32_t) 0xfe);
				LOG_INF("initiator -> responder Distance: %3.2lf \n", distance);
				send_twr_notify(responder_id);
			} else {
				LOG_WRN("Something is wrong");
				dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
			}
		}
		sequence++;
		k_msleep(500);
	}
}

void sit_sstwr_responder() {
	while(device_settings.state == measurement) {
		sit_receive_at(0);
		msg_simple_t rx_poll_msg;
		msg_id_t msg_id = twr_1_poll;
		if(sit_check_msg_id(msg_id, &rx_poll_msg)){
			uint64_t poll_rx_ts = get_rx_timestamp_u64();
			
			uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

			uint16_t tx_dly = get_tx_ant_dly();
			LOG_INF("TX Antenna Delay: %d", tx_dly);
			uint32_t resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + tx_dly;
			
			msg_ss_twr_final_t msg_ss_twr_final_t = {{
					ss_twr_2_resp,
					(uint8_t)(rx_poll_msg.header.sequence),
					device_settings.deviceID, 
					rx_poll_msg.header.source},
					(uint32_t)poll_rx_ts, 
					resp_tx_ts,
					0
				};
			sit_send_at((uint8_t*)&msg_ss_twr_final_t, sizeof(msg_ss_twr_final_t), resp_tx_time);

		} else {
			LOG_WRN("Something is wrong");
			dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		}
		k_msleep(90);
	}
}

void sit_dstwr_initiator() {
	while(device_settings.state == measurement) {
		for(uint8_t responder_id=100; responder_id<=device_settings.responder; responder_id++) {
			sit_set_rx_after_tx_delay(DS_POLL_TX_TO_RESP_RX_DLY_UUS);
			sit_set_rx_timeout(DS_RESP_RX_TIMEOUT_UUS);
			sit_set_preamble_detection_timeout(DS_PRE_TIMEOUT);

			msg_simple_t twr_poll = {{twr_1_poll, sequence, device_settings.deviceID , responder_id},0};
			sit_start_poll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));

			msg_simple_t rx_resp_msg;
			msg_id_t msg_id = ds_twr_2_resp;

			if(sit_check_msg_id(msg_id, &rx_resp_msg)) {
				uint64_t poll_tx_ts = get_tx_timestamp_u64();
				uint64_t resp_rx_ts = get_rx_timestamp_u64();
				
				uint32_t final_tx_time = (resp_rx_ts + (DS_RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
				uint64_t final_tx_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFEUL)) << 8) + get_tx_ant_dly();

				msg_ds_twr_final_t final_msg = {{
					ds_twr_3_final,
					rx_resp_msg.header.sequence,
					rx_resp_msg.header.dest,
					rx_resp_msg.header.source},
					(uint32_t)poll_tx_ts,
					(uint32_t)resp_rx_ts,
					(uint32_t)final_tx_ts,
					0
				};

				bool ret = sit_send_at((uint8_t*)&final_msg, sizeof(msg_ds_twr_final_t),final_tx_time);

				if (ret == false) {
					LOG_WRN("Something is wrong with Sending Final Msg");
					continue;
				}
			} else {
				LOG_WRN("Something is wrong with Receiving Msg");
				dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
			}
		}
		sequence++;
		k_msleep(100);
	}
}

void sit_dstwr_responder() {
	while(device_settings.state == measurement) { 
		sit_receive_now();
		msg_simple_t rx_poll_msg;
		msg_id_t msg_id = twr_1_poll;
		if(sit_check_msg_id(msg_id, &rx_poll_msg) && rx_poll_msg.header.dest == device_settings.deviceID){
			uint64_t poll_rx_ts = get_rx_timestamp_u64();
				
			uint32_t resp_tx_time = (poll_rx_ts + (DS_POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
			
			msg_simple_t msg_ds_poll_resp = {{
					ds_twr_2_resp,
					rx_poll_msg.header.sequence,
					rx_poll_msg.header.dest,
					rx_poll_msg.header.source,
				},0};
			sit_set_rx_after_tx_delay(DS_RESP_TX_TO_FINAL_RX_DLY_UUS);
			sit_set_rx_timeout(DS_FINAL_RX_TIMEOUT);
			sit_set_preamble_detection_timeout(DS_PRE_TIMEOUT);
			bool ret = sit_send_at_with_response((uint8_t*)&msg_ds_poll_resp, sizeof(msg_simple_t), resp_tx_time);
			if (ret == false) {
				continue;
				LOG_WRN("Something is wrong with Sending Poll Resp Msg");
				k_msleep(90);
			}
			msg_ds_twr_final_t rx_ds_final_msg;
			msg_id = ds_twr_3_final;
			if(sit_check_ds_final_msg_id(msg_id, &rx_ds_final_msg) && rx_ds_final_msg.header.dest == device_settings.deviceID){
				uint64_t resp_tx_ts = get_tx_timestamp_u64();
				uint64_t final_rx_ts = get_rx_timestamp_u64();

				uint32_t poll_tx_ts = rx_ds_final_msg.poll_tx_ts;
				uint32_t resp_rx_ts = rx_ds_final_msg.resp_rx_ts;
				uint32_t final_tx_ts = rx_ds_final_msg.final_tx_ts;

				uint32_t poll_rx_ts_32, resp_tx_ts_32, final_rx_ts_32;
				poll_rx_ts_32 = (uint32_t) poll_rx_ts;
				resp_tx_ts_32 = (uint32_t) resp_tx_ts;
				final_rx_ts_32 = (uint32_t) final_rx_ts;

				int64_t tof_dtu;
				time_round_1 = (double)(resp_rx_ts - poll_tx_ts);
				time_round_2 = (double)(final_rx_ts_32 - resp_tx_ts_32);
				time_reply_1 = (double)(final_tx_ts - resp_rx_ts);
				time_reply_2 = (double)(resp_tx_ts_32 - poll_rx_ts_32);
				tof_dtu = (int64_t)((time_round_1 * time_round_2 - time_reply_1 * time_reply_2) 
										/ (time_round_1 + time_round_2 + time_reply_1 + time_reply_2)
									);

				double tof = (double)tof_dtu * DWT_TIME_UNITS;
				distance = tof * SPEED_OF_LIGHT;
				LOG_INF("Distance: %lf", distance);
				send_twr_notify(device_settings.deviceID);
			} else {
                LOG_WRN("Something is wrong with Final Msg Receive");
                dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            }

		} else {
			LOG_WRN("Something is wrong with Poll Msg Receive");
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		}
		sequence++;
		k_msleep(90);
	}
}

// void sit_simple_calibration_a(){
// 	LOG_INF("Simple Calibration A");
// 	sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
// 	msg_simple_t simple_poll_msg = {simple_poll, 0, device_settings.deviceID, 1, 0};
// 	sit_start_poll((uint8_t*) &simple_poll_msg, (uint16_t)sizeof(simple_poll_msg));
// 	if (device_settings.measurement_type == extended_calibration) {
// 		msg_simple_t resp_msg;
// 		if (sit_check_msg_id(simple_resp, &resp_msg)) {
// 			simple_poll_msg.header.id = extended_poll;
// 			sit_start_poll((uint8_t*) &simple_poll_msg, (uint16_t)sizeof(simple_poll_msg));
// 		}
// 		k_msleep(100);
// 	} else {
// 		k_msleep(1000);
	
// 	}
	
// }

// void sit_simple_calibration_b(){
// 	LOG_INF("Simple Calibration B");
// 	sit_receive_now();
// 	msg_simple_t simple_poll_msg;
// 	msg_id_t msg_id = simple_poll;
// 	uint64_t poll_rx, resp_tx, final_rx = 0;
// 	if(sit_check_msg_id(msg_id, &simple_poll_msg)){
// 		LOG_INF("Simple Poll B");
// 		uint64_t poll_rx = get_rx_timestamp_u64();
// 		uint32_t resp_tx_time = (poll_rx + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

// 		uint16_t tx_dly = get_tx_ant_dly();
// 		uint32_t resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + tx_dly;
// 		msg_id = simple_resp;
// 		msg_simple_t simple_resp_msg = {simple_resp, (uint8_t)sequence, device_settings.deviceID, 0, 0};
// 		sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
// 		sit_start_poll((uint8_t*) &simple_resp_msg, (uint16_t)sizeof(simple_resp_msg));
// 		if (device_settings.measurement_type == extended_calibration) {
// 			msg_id = extended_poll;
// 			if(sit_check_msg_id(msg_id, &simple_poll_msg)) {
// 				final_rx = get_rx_timestamp_u64();
// 			}
// 		}
// 		msg_id = simple_final;
// 		simple_calibration_t final_msg;
// 		if (sit_check_simple_cali_final_msg_id(msg_id, &final_msg)) {
// 			LOG_INF("Simple Final B");
// 			time_tc_i = (double) final_msg.resp_rx - final_msg.poll_rx;
			
// 			time_tb_i = (double) resp_tx - poll_rx;

// 			if (device_settings.measurement_type == extended_calibration) {
// 				time_tc_ii = (double) final_msg.final_rx - final_msg.resp_rx;
// 				time_tb_ii = (double) final_rx - resp_tx;
// 			}
// 			send_simple_notify();

// 			k_msleep(90);
// 		}
// 	}
// }

// void sit_simple_calibration_c(){
// 	LOG_INF("Simple Calibration C");
// 	sit_receive_now();
// 	msg_simple_t simple_poll_msg;
// 	msg_id_t msg_id = simple_poll;
// 	uint64_t poll_rx, resp_rx, final_rx = 0;
// 	uint32_t resp_tx_time = 0;
// 	if(sit_check_msg_id(msg_id, &simple_poll_msg)) {
// 		LOG_INF("Simple Poll C");
// 		poll_rx = get_rx_timestamp_u64();
// 		sit_receive_now(); 
// 		msg_id = simple_resp;
// 		if(sit_check_msg_id(msg_id, &simple_poll_msg)) {
// 			LOG_INF("Simple Resp C");
// 			resp_rx = get_rx_timestamp_u64();
// 			resp_tx_time = (resp_rx + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
// 		}
// 		if(device_settings.measurement_type == extended_calibration) {
// 			sit_receive_now();
// 			msg_id = simple_final;
// 			if(sit_check_msg_id(msg_id, &simple_poll_msg)) {
// 				final_rx = get_rx_timestamp_u64();
// 			}
// 			resp_tx_time = (final_rx + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

// 		}
// 		sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
// 		simple_calibration_t simple_final_msg = {
// 			simple_final, 
// 			(uint8_t)sequence,
// 			device_settings.deviceID,
// 			1,
// 			(uint32_t)poll_rx,
// 			(uint32_t)resp_rx,
// 			(uint32_t)final_rx,
// 			0
// 		};
// 		sit_send_at((uint8_t*)&simple_final_msg, sizeof(simple_final_msg), resp_tx_time);
// 		k_msleep(90);
// 	}
// }

// void sit_two_device_calibration_a() {
// 	while(device_settings.state == measurement) {
// 		uint64_t sensing_1_tx, sensing_2_rx, sensing_3_tx = 0;
// 		sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
// 		msg_simple_t sensing_1_msg = {sensing_1, (uint8_t)sequence, device_settings.deviceID, 1, 0};
// 		sit_start_poll((uint8_t*) &sensing_1_msg, (uint16_t)sizeof(sensing_1_msg));

// 		msg_simple_t resp_msg;
// 		if (sit_check_msg_id(sensing_2, &resp_msg)) {
// 			sensing_1_tx = get_tx_timestamp_u64();
// 			sensing_2_rx = get_rx_timestamp_u64();

// 			uint32_t sensing_3_tx_time = (sensing_2_rx + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

// 			uint16_t tx_dly = get_tx_ant_dly();
// 			sensing_3_tx = (((uint64_t)(sensing_3_tx_time & 0xFFFFFFFEUL)) << 8) + tx_dly;


// 			msg_sensing_3_t sensing_3_msg = {
// 				sensing_3,
// 				(uint8_t)sequence,
// 				device_settings.deviceID,
// 				2,
// 				(uint32_t)sensing_1_tx,
// 				(uint32_t)sensing_2_rx,
// 				(uint32_t)sensing_3_tx,
// 				0
// 			};
// 			sit_send_at((uint8_t*) &sensing_3_msg, (uint16_t)sizeof(sensing_3_msg), sensing_3_tx_time);
// 			msg_sensing_info_t info_msg;
// 			if(sit_check_sensing_info_msg_id(simple_resp, &info_msg)){
// 				k_msleep(100);
// 			} else {
// 				LOG_WRN("Something is wrong");
// 				dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
// 				k_msleep(500);
// 			}
// 		} else {
// 				LOG_WRN("Something is wrong");
// 				dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
// 		}
// 	}
// }

// void sit_two_device_calibration_b() {
// 	while(device_settings.state == measurement) {
// 		sit_receive_now();
// 		msg_simple_t sensing_1_msg;
// 		msg_id_t msg_id = sensing_1;
// 		uint64_t sensing_1_rx, sensing_2_tx, sensing_3_rx = 0;
// 		if(sit_check_msg_id(msg_id, &sensing_1_msg)){
// 			sensing_1_rx = get_rx_timestamp_u64();
// 			uint32_t sesing_2_tx_time = (sensing_1_rx + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

// 			uint16_t tx_dly = get_tx_ant_dly();
// 			sensing_2_tx = (((uint64_t)(sesing_2_tx_time & 0xFFFFFFFEUL)) << 8) + tx_dly;
// 			msg_simple_t sensing_2_msg = {sensing_2, (uint8_t)sequence, device_settings.deviceID, 0, 0};
// 			sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
// 			sit_start_poll((uint8_t*) &sensing_2_msg, (uint16_t)sizeof(sensing_2_msg));
// 			msg_id = sensing_3;
// 			msg_sensing_3_t sensing_3_msg;
// 			if (sit_check_sensing_3_msg_id(msg_id, &sensing_3_msg) ){
// 				sensing_3_rx = get_rx_timestamp_u64();
// 				sit_receive_now();
// 				msg_sensing_info_t sensing_info_msg;
// 				if (sit_check_sensing_info_msg_id(sensing_resp, &sensing_info_msg)) {
// 					time_m21 = (double) sensing_3_msg.sensing_2_rx - sensing_3_msg.sensing_1_tx;
// 					time_m31 = (double) sensing_3_msg.sensing_3_tx - sensing_3_msg.sensing_1_tx;

// 					time_a21 = (double) sensing_2_tx - sensing_1_rx;
// 					time_a31 = (double) sensing_3_rx - sensing_1_rx;

// 					time_b21 = (double) sensing_info_msg.sensing_2_rx - sensing_info_msg.sensing_1_rx;
// 					time_b31 = (double) sensing_info_msg.sensing_3_rx - sensing_info_msg.sensing_1_rx;
					
// 				} 
// 			}
// 		}
// 		k_msleep(50);
// 	}
// }

// void sit_two_device_calibration_c() {
// 	while(device_settings.state == measurement) {
// 		sit_receive_now();
// 		msg_simple_t simple_poll_msg;
// 		msg_id_t msg_id = sensing_1;
// 		uint64_t sensing_1_rx, sensing_2_rx, sensing_3_rx = 0;
// 		if(sit_check_msg_id(msg_id, &simple_poll_msg) && simple_poll_msg.header.dest == 1){
// 			sensing_1_rx = get_rx_timestamp_u64();
// 			sit_receive_now();
// 			msg_id = sensing_2;
// 			if(sit_check_msg_id(msg_id, &simple_poll_msg)){
// 				sensing_2_rx = get_rx_timestamp_u64();
// 				sit_receive_now();
// 				msg_id = sensing_3;
// 				msg_sensing_3_t sensing_3_msg;
// 				if(sit_check_sensing_3_msg_id(msg_id, &sensing_3_msg)) {
// 					sensing_3_rx = get_rx_timestamp_u64();
// 					uint32_t sesing_3_tx_time = (sensing_3_rx + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
// 					msg_sensing_info_t sensing_info = {
// 						sensing_resp,
// 						(uint8_t)sequence,
// 						device_settings.deviceID,
// 						0,
// 						(uint32_t)sensing_1_rx,
// 						(uint32_t)sensing_2_rx,
// 						(uint32_t)sensing_3_rx,
// 						0
// 					};

// 					sit_send_at((uint8_t*)&sensing_info, sizeof(sensing_info), sesing_3_tx_time);
// 					} else {
// 						LOG_WRN("Something is wrong");
// 						dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
// 					}
// 				}
// 			}
// 		k_msleep(50);
// 	}
// }


uint8_t sit_init() {
	device_init();
	/* Configure SPI rate, for initialize it should not faster than 7 MHz */
	port_set_dw_ic_spi_slowrate();
	
	/* Reset and initialize DW chip. */
	reset_DWIC(); /* Target specific drive of RSTn line into DW3000 low for a period. */

	k_msleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)
	
	/* Configure SPI rate, after init up to 36 MHz */
	port_set_dw_ic_spi_fastrate();

	/* Probe set device speific functions (eg. write and read spi). */
	if (dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf) == DWT_ERROR) {
		LOG_ERR("dwt_probe failed");
		return -3;
	}

	/* Need to make sure DW IC is in IDLE_RC before proceeding */
	while (!dwt_checkidlerc()){
	};
	/* Configure SPI rate, DW3000 supports up to 36 MHz */
	port_set_dw_ic_spi_fastrate();
	if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR) {
		LOG_ERR("dwt_initialise failed");
		return -1;
	}
	/* Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards. */
	dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

	/* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
	if (dwt_configure(&sit_device_config)){
		LOG_ERR("dwt_configure failed");
		return -2;
	}
	/* Configure the TX spectrum parameters (power, PG delay and PG count) */
	dwt_configuretxrf(&txconfig_options_ch9_sit);

	set_antenna_delay(device_settings.rx_ant_dly, device_settings.tx_ant_dly);

	/* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
	 * Note, in real low power applications the LEDs should not be used. */
	dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

	/* Enable Diacnostic all */
	dwt_configciadiag(DW_CIA_DIAG_LOG_ALL);
 	k_msleep(100);

	return 1;
}

void sit_run_forever(){
	ble_start_connection();
	while(42) { //Life, the universe, and everything
		if(is_connected()){
			if (device_settings.measurement_type == ss_twr && device_type == initiator) {
					sit_sstwr_initiator();
			} else if (device_settings.measurement_type == ss_twr && device_type == responder) {
					sit_sstwr_responder();
			} else if (device_settings.measurement_type == ds_3_twr && device_type == initiator) {
					sit_dstwr_initiator();
			} else if (device_settings.measurement_type == ds_3_twr && device_type == responder) {
					sit_dstwr_responder();
			// } else if  (device_settings.measurement_type == simple_calibration && device_type == A) {
			// 		sit_simple_calibration_a();
			// } else if  (device_settings.measurement_type == simple_calibration && device_type == B) {
			// 		sit_simple_calibration_b();
			// } else if  (device_settings.measurement_type == simple_calibration && device_type == C) {
			// 		sit_simple_calibration_c();
			// } else if  (device_settings.measurement_type == two_device_calibration && device_type == A) {
			// 		sit_two_device_calibration_a();
			// } else if  (device_settings.measurement_type == two_device_calibration && device_type == B) {
			// 		sit_two_device_calibration_b();
			// } else if  (device_settings.measurement_type == two_device_calibration && device_type == C) {
			// 		sit_two_device_calibration_c();
			}
		} else {
			ble_wait_for_connection();
		}
		k_msleep(100);
	}
}