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

#define DGC_CFG_ID 0x03

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
double time_round_1 = 0.0, time_round_2 = 0.0;
double time_reply_1 = 0.0, time_reply_2 = 0.0;
double time_tc_i = 0.0, time_tc_ii = 0.0;
double time_tb_i = 0.0, time_tb_ii = 0.0;
double time_m21 = 0.0, time_m31 = 0.0; 
double time_a21 = 0.0, time_a31 = 0.0;
double time_b21 = 0.0, time_b31 = 0.0;


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
		LOG_INF("Responder: %d", responder);
		json_distance_msg_all_t distance_notify = {
			.header = {
				.type = "distance_msg",
				.state = "running",
				.responder = responder,
				.sequence = sequence,
				.measurements = measurements,
			},
			.data = {
				.distance = distance,
				.time_round_1 = (float)(time_round_1 * DWT_TIME_UNITS),
				.time_round_2 = (float)(time_round_2 * DWT_TIME_UNITS),
				.time_reply_1 = (float)(time_reply_1 * DWT_TIME_UNITS),
				.time_reply_2 = (float)(time_reply_2 * DWT_TIME_UNITS),
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
		LOG_INF("Test Measurement: %d von %d", measurements, device_settings.max_measurement);
		if(device_settings.max_measurement != 0 && device_settings.max_measurement <= measurements) {
			device_settings.state = sleep;
		}
	}
}

void send_two_device_notify() {
	json_simple_td_msg_t distance_notify = {
		.header = {
			.type = "cali_msg",
			.sequence = sequence,
			.measurements = measurements,
		},
		.data = {
			.time_m21 = (float)(time_m21 * DWT_TIME_UNITS),
			.time_m31 = (float)(time_m31 * DWT_TIME_UNITS),
			.time_a21 = (float)(time_a21 * DWT_TIME_UNITS),
			.time_a31 = (float)(time_a31 * DWT_TIME_UNITS),
			.time_b21 = (float)(time_b21 * DWT_TIME_UNITS),
			.time_b31 = (float)(time_b31 * DWT_TIME_UNITS),
			.time_tc_i = (float)(time_tc_i * DWT_TIME_UNITS),
			.time_tc_ii = (float)(time_tc_ii * DWT_TIME_UNITS),
			.time_tb_i = (float)(time_tb_i * DWT_TIME_UNITS),
			.time_tb_ii = (float)(time_tb_ii * DWT_TIME_UNITS),
			.time_round_1 = (float)(time_round_1 * DWT_TIME_UNITS),
			.time_round_2 = (float)(time_round_2 * DWT_TIME_UNITS),
			.time_reply_1 = (float)(time_reply_1 * DWT_TIME_UNITS),
			.time_reply_2 = (float)(time_reply_2 * DWT_TIME_UNITS),
			.distance = (float)distance,
			.dummy = 0,
		}
	};
	ble_sit_td_notify(&distance_notify, sizeof(distance_notify));
	measurements++;
	if(device_settings.max_measurement != 0 && device_settings.max_measurement <= measurements) {
		device_settings.state = sleep;
	}
}

void sit_sstwr_initiator() {
	while(device_settings.state == measurement) {
		sit_set_rx_after_tx_delay(DS_RESP_TX_TO_FINAL_RX_DLY_UUS);
		sit_set_rx_timeout(DS_FINAL_RX_TIMEOUT+2000);
		sit_set_preamble_detection_timeout(DS_PRE_TIMEOUT+200);
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

				time_round_1 = (double)(resp_rx_ts - poll_tx_ts);
				time_reply_1 = (double)(resp_tx_ts - poll_rx_ts);

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
		k_msleep(100);
	}
}

void sit_sstwr_responder() {
	while(device_settings.state == measurement) {
		sit_receive_now(0,0);
		msg_simple_t rx_poll_msg;
		msg_id_t msg_id = twr_1_poll;
		if(sit_check_msg_id(msg_id, &rx_poll_msg)){
			uint64_t poll_rx_ts = get_rx_timestamp_u64();
			
			uint32_t resp_tx_time = (poll_rx_ts + (1800 * UUS_TO_DWT_TIME)) >> 8;

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
			sit_set_rx_timeout(DS_RESP_RX_TIMEOUT_UUS+2000);
			sit_set_preamble_detection_timeout(DS_PRE_TIMEOUT+200);

			msg_simple_t twr_poll = {{twr_1_poll, sequence, device_settings.deviceID , responder_id},0};
			sit_start_poll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));

			msg_simple_t rx_resp_msg;
			msg_id_t msg_id = ds_twr_2_resp;

			if(sit_check_msg_id(msg_id, &rx_resp_msg)) {
				uint64_t poll_tx_ts = get_tx_timestamp_u64();
				uint64_t resp_rx_ts = get_rx_timestamp_u64();
				
				uint32_t final_tx_time = (resp_rx_ts + (1800 * UUS_TO_DWT_TIME)) >> 8;
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
		sit_receive_now(0,0);
		msg_simple_t rx_poll_msg;
		msg_id_t msg_id = twr_1_poll;
		if(sit_check_msg_id(msg_id, &rx_poll_msg) && rx_poll_msg.header.dest == device_settings.deviceID){
			uint64_t poll_rx_ts = get_rx_timestamp_u64();
				
			uint32_t resp_tx_time = (poll_rx_ts + (1800 * UUS_TO_DWT_TIME)) >> 8;
			
			msg_simple_t msg_ds_poll_resp = {{
					ds_twr_2_resp,
					rx_poll_msg.header.sequence,
					rx_poll_msg.header.dest,
					rx_poll_msg.header.source,
				},0};
			sit_set_rx_after_tx_delay(DS_RESP_TX_TO_FINAL_RX_DLY_UUS);
			sit_set_rx_timeout(DS_FINAL_RX_TIMEOUT+2000);
			sit_set_preamble_detection_timeout(DS_PRE_TIMEOUT+200);
			bool ret = sit_send_at_with_response((uint8_t*)&msg_ds_poll_resp, sizeof(msg_simple_t), resp_tx_time);
			if (ret == false) {
				continue;
				LOG_WRN("Something is wrong with Sending Poll Resp Msg");
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
				time_reply_1 = (double)(resp_tx_ts_32 - poll_rx_ts_32);
				time_reply_2 = (double)(final_tx_ts - resp_rx_ts);
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

void sit_two_device_calibration_a() {
	while(device_settings.state == measurement) {
		uint64_t sensing_1_tx, sensing_2_rx, sensing_3_tx = 0;
		LOG_INF("Two Device Calibration A: %d", sequence);
		sit_set_rx_after_tx_delay(POLL_TX_TO_RESP_RX_DLY_UUS);
		sit_set_rx_timeout(DS_RESP_RX_TIMEOUT_UUS+2000);
		sit_set_preamble_detection_timeout(DS_PRE_TIMEOUT+200);
		msg_simple_t sensing_1_msg = {{sensing_1, (uint8_t)sequence, device_settings.deviceID, 1}, 0};
		sit_start_poll((uint8_t*) &sensing_1_msg, (uint16_t)sizeof(sensing_1_msg));

		msg_simple_t resp_msg;
		if (sit_check_msg_id(sensing_2, &resp_msg)) {
			LOG_INF("Sensing 2 A");
			sensing_1_tx = get_tx_timestamp_u64();
			sensing_2_rx = get_rx_timestamp_u64();

			uint32_t sensing_3_tx_time = (sensing_2_rx + (1800 * UUS_TO_DWT_TIME)) >> 8;

			sensing_3_tx = (((uint64_t)(sensing_3_tx_time & 0xFFFFFFFEUL)) << 8) + get_tx_ant_dly();

			msg_sensing_3_t sensing_3_msg = {{
				sensing_3,
				(uint8_t)sequence,
				device_settings.deviceID,
				2},
				(uint32_t)sensing_1_tx,
				(uint32_t)sensing_2_rx,
				(uint32_t)sensing_3_tx,
				0
			};
			bool ret = sit_send_at_with_response((uint8_t*) &sensing_3_msg, (uint16_t)sizeof(sensing_3_msg), sensing_3_tx_time);
			if (ret == false) {
				LOG_WRN("Something is wrong with Sending Sennsing 3 Msg");
				continue;
			}
			msg_sensing_info_t info_msg;
			if(sit_check_sensing_info_msg_id(sensing_resp, &info_msg)){
				LOG_INF("Sensing Info Final A");
			}
		}
		sequence++;
		k_msleep(1000);
	}
	LOG_INF("Simple Calibration Test");
}

void sit_two_device_calibration_b() {
	while(device_settings.state == measurement) {
		LOG_INF("Two Device Calibration B: %d", sequence);
		sit_receive_now(0,0);
		msg_simple_t sensing_1_msg;
		uint64_t sensing_1_rx, sensing_2_tx, sensing_3_rx = 0;
		if(sit_check_msg_id(sensing_1, &sensing_1_msg)){
			LOG_INF("Sensing 1 B");
			sensing_1_rx = get_rx_timestamp_u64();
			uint32_t sesing_2_tx_time = (sensing_1_rx + (1800 * UUS_TO_DWT_TIME)) >> 8;

			sit_set_rx_after_tx_delay(1500);
			sit_set_rx_timeout(DS_RESP_RX_TIMEOUT_UUS+2000);
			sit_set_preamble_detection_timeout(DS_PRE_TIMEOUT+200);			
			msg_simple_t sensing_2_msg = {{sensing_2, (uint8_t)sequence, device_settings.deviceID, 0}, 0};
			sit_send_at_with_response((uint8_t*) &sensing_2_msg, (uint16_t)sizeof(sensing_2_msg),sesing_2_tx_time);
			msg_sensing_3_t resp_sensing_3;
			if (sit_check_sensing_3_msg_id(sensing_3, &resp_sensing_3) ){
				LOG_INF("Sensing 3 B");
				sensing_2_tx = get_tx_timestamp_u64();
				sensing_3_rx = get_rx_timestamp_u64();

				uint32_t sesing_3_tx_time = (sensing_3_rx + (1800 * UUS_TO_DWT_TIME)) >> 8;
				msg_sensing_info_t sensing_info = {{
					sensing_resp,
					(uint8_t)sequence,
					device_settings.deviceID,
					0},
					(uint32_t)sensing_1_rx,
					(uint32_t)sensing_2_tx,
					(uint32_t)sensing_3_rx,
					0
				};

				sit_send_at((uint8_t*)&sensing_info, sizeof(sensing_info), sesing_3_tx_time);

			}
		}
		sequence++;
		k_msleep(500);
	}
	LOG_INF("Simple Calibration Test");
	k_msleep(500);
}

void sit_two_device_calibration_c() {
	while(device_settings.state == measurement) {
		LOG_INF("Two Device Calibration C: %d", sequence);
		sit_receive_now(0,0);
		msg_simple_t simple_poll_msg;
		uint64_t sensing_1_rx, sensing_2_rx, sensing_3_rx = 0;
		if(sit_check_msg_id(sensing_1, &simple_poll_msg)){
			LOG_INF("Sensing 1 C");
			sensing_1_rx = get_rx_timestamp_u64();
			sit_receive_now(DS_PRE_TIMEOUT+200, DS_RESP_RX_TIMEOUT_UUS+2000);
			if(sit_check_msg_id(sensing_2, &simple_poll_msg)){
				LOG_INF("Sensing 2 C");
				sensing_2_rx = get_rx_timestamp_u64();
				sit_receive_now(DS_PRE_TIMEOUT+200, DS_RESP_RX_TIMEOUT_UUS+2000);
				msg_sensing_3_t sensing_3_msg;
				if(sit_check_sensing_3_msg_id(sensing_3, &sensing_3_msg)){
					LOG_INF("Sensing 3 C");
					sensing_3_rx = get_rx_timestamp_u64();
					sit_receive_now(DS_PRE_TIMEOUT+200, DS_RESP_RX_TIMEOUT_UUS+2000);
					msg_sensing_info_t sensing_info_msg;
					if(sit_check_sensing_info_msg_id(sensing_resp, &sensing_info_msg)){
						LOG_INF("Sensing Info Final C");
						
							time_m21 = (double) (sensing_3_msg.sensing_2_rx - sensing_3_msg.sensing_1_tx);
							time_m31 = (double) (sensing_3_msg.sensing_3_tx - sensing_3_msg.sensing_1_tx);

							time_a21 = (double) (sensing_info_msg.sensing_2_tx - sensing_info_msg.sensing_1_rx);
							time_a31 = (double) (sensing_info_msg.sensing_3_rx - sensing_info_msg.sensing_1_rx);

							time_b21 = (double) (sensing_2_rx - sensing_1_rx);
							time_b31 = (double) (sensing_3_rx - sensing_1_rx);

							time_tb_i = (double) (sensing_info_msg.sensing_2_tx - sensing_info_msg.sensing_1_rx);
							time_tb_ii = (double) (sensing_info_msg.sensing_3_rx - sensing_info_msg.sensing_2_tx);

							time_tc_i = (double) (sensing_2_rx - sensing_1_rx);
							time_tc_ii = (double) (sensing_3_rx - sensing_2_rx);

							time_round_1 = (double) (sensing_3_msg.sensing_2_rx - sensing_3_msg.sensing_1_tx);
							time_round_2 = (double) (sensing_info_msg.sensing_3_rx - sensing_info_msg.sensing_2_tx);
							time_reply_1 = (double) (sensing_info_msg.sensing_2_tx - sensing_info_msg.sensing_1_rx);
							time_reply_2 = (double) (sensing_3_msg.sensing_3_tx - sensing_3_msg.sensing_2_rx);

							uint64_t tof_dtu;
							tof_dtu = (uint64_t)((time_round_1 * time_round_2 - time_reply_1 * time_reply_2) 
										/ (time_round_1 + time_round_2 + time_reply_1 + time_reply_2)
									);

							double tof = (double)tof_dtu * DWT_TIME_UNITS;
							distance = tof * SPEED_OF_LIGHT;

							send_two_device_notify();
						} 
				}
			}
		}
		sequence++;
		k_msleep(500);
	}
	LOG_INF("Simple Calibration Test");
}


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
					sit_dstwr_initiator();
			} else if (device_settings.measurement_type == ss_twr && device_type == responder) {
					sit_dstwr_responder();
			} else if (device_settings.measurement_type == ds_3_twr && device_type == initiator) {
					sit_dstwr_initiator();
			} else if (device_settings.measurement_type == ds_3_twr && device_type == responder) {
					sit_dstwr_responder();
			} else if  (device_settings.measurement_type == two_device_calibration && device_type == dev_a) {
					sit_two_device_calibration_a();
			} else if  (device_settings.measurement_type == two_device_calibration && device_type == dev_b) {
					sit_two_device_calibration_b();
			} else if  (device_settings.measurement_type == two_device_calibration && device_type == dev_c) {
					sit_two_device_calibration_c();
			}
		} else {
			ble_wait_for_connection();
		}
		k_msleep(100);
	}
}