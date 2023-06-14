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
#include <sit_json/sit_json.h>
#include <sit_ble/ble_init.h>

#include <stdlib.h>
#include <string.h>

#include <deca_probe_interface.h>
#include <deca_device_api.h>
#include <port.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SIT_Module, LOG_LEVEL_INF);

static uint32_t status_reg = 0;

extern dwt_config_t sit_device_config;
static char deviceID[17];

json_distance_msg_t distance_notify = {
	.type = "distance_msg",
	.data = {
		.state = "running",
		.sequence = 0,
		.distance = 0,
		.nlos_percent = 0,
		.rssi_index_resp = 0,
		.fp_index_resp = 0
	}
};
/**
 * Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. 
 */
dwt_txconfig_t txconfig_options_ch9_sit = {
    0x34,       /* PG delay. */
    0xfefefefe, /* TX power. */
    0x0         /*PG count*/
};

uint16_t antennadely = 16385;

uint32_t sequence = 0;

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

	set_antenna_delay(antennadely, antennadely);

	/* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
	 * Note, in real low power applications the LEDs should not be used. */
	dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

	/* Enable Diacnostic all */
	dwt_configciadiag(DW_CIA_DIAG_LOG_ALL);

	return 1;
}

void sit_sstwr_initiator(uint8_t initiator_node_id, uint8_t responder_node_id) {
	sit_set_rx_tx_delay_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
	msg_header_t twr_poll = {twr_1_poll, (uint8_t)sequence, initiator_node_id , responder_node_id, 0};
	sit_start_poll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));

	msg_ss_twr_final_t rx_final_msg;
    msg_id_t msg_id = ss_twr_2_resp;
	if(sit_check_final_msg_id(msg_id, &rx_final_msg)) {
		uint64_t poll_tx_ts = get_tx_timestamp_u64();
		uint64_t resp_rx_ts = get_rx_timestamp_u64();
		
		uint64_t poll_rx_ts = rx_final_msg.poll_rx_ts;
		uint64_t resp_tx_ts = rx_final_msg.resp_tx_ts;

		float clockOffsetRatio = dwt_readcarrierintegrator() * 
				(FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_9 / 1.0e6) ;

		uint64_t rtd_init = resp_rx_ts - poll_tx_ts;
		uint64_t rtd_resp = resp_tx_ts - poll_rx_ts;

		float tof =  ((rtd_init - rtd_resp * 
				(1 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;
		
		float distance = tof * SPEED_OF_LIGHT;

		LOG_INF("TX Power: 0x%08x", (int32_t) 0xfe);
		LOG_INF("initiator -> responder Distance: %3.2lf \n", distance);
		if (distance >= 0) {
			distance_notify.data.distance = distance;
			distance_notify.data.sequence = sequence;
			distance_notify.data.nlos_percent = diagnostic.nlos;
			distance_notify.data.rssi_index_resp = diagnostic.rssi;
			distance_notify.data.fp_index_resp = diagnostic.fpi;
			ble_sit_notify(&distance_notify, sizeof(distance_notify));
		}
	} else {
		LOG_WRN("Something is wrong");
		dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
	}
	sequence++;
}

void sit_responder() {
	sit_receive_at(0);
    msg_header_t rx_poll_msg;
	msg_id_t msg_id = twr_1_poll;
	if(sit_check_msg_id(msg_id, &rx_poll_msg)){
		uint64_t poll_rx_ts = get_rx_timestamp_u64();
		
		uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

		uint16_t tx_dly = get_rx_ant_dly();

		uint32_t resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + 16385;
		
		msg_ss_twr_final_t msg_ss_twr_final_t = {
				ss_twr_2_resp,
				(uint8_t)(rx_poll_msg.header.sequence),
				rx_poll_msg.header.dest, 
				rx_poll_msg.header.source,
				(uint32_t)poll_rx_ts, 
				resp_tx_ts,
				0
			};
		sit_send_at((uint8_t*)&msg_ss_twr_final_t, sizeof(msg_ss_twr_final_t), resp_tx_time);

	} else {
		LOG_WRN("Something is wrong");
        dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
	}
}

void reset_sequence() {
	sequence = 0;
}
