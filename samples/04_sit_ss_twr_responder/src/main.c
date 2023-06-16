/**
 * Copyright (c) 2019 - Frederic Mes, RTLOC
 * Copyright (c) 2015 - Decawave Ltd, Dublin, Ireland.
 * Copyright (c) 2021 - Home Smart Mesh
 * Copyright (c) 2022 - Sven Hoyer
 * 
 * This file is part of Zephyr-DWM1001.
 *
 *   Zephyr-DWM1001 is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Zephyr-DWM1001 is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Zephyr-DWM1001.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */
#include "deca_probe_interface.h"
#include <config_options.h>
#include <deca_device_api.h>
#include <dw3000_hw.h> 
#include <deca_spi.h>
#include <example_selection.h>
#include <port.h>
#include <shared_defines.h>

#include <sit.h>
#include <sit_led.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define APP_NAME "SIMPLE TWR Responder EXAMPLE\n"

/* Inter-ranging delay period, in milliseconds. */
#define RNG_DELAY_MS 1000

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    5,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    DWT_SFD_DW_8,     /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

uint8_t this_initiator_node_id  = 1;
uint8_t responder_node_id       = 2;

/*rx twr_2_resp after tx twr_1_poll
 protected by responder's mp_request_at(twr_2_resp):POLL_RX_TO_RESP_TX_DLY_UUS
*/
#define POLL_TX_TO_RESP_RX_DLY_UUS 500

/* Delay between frames, in UWB microseconds. See NOTE 1 below. */
#define POLL_RX_TO_RESP_TX_DLY_UUS 650

int main(void) {
	printk(APP_NAME);
	printk("==================\n");

	int init_ok = sit_init(&config, TX_ANT_DLY, RX_ANT_DLY);
	// INIT LED and let them Blink one Time to see Intitalion Finshed
    sit_led_init();
    
    if(init_ok < 0){
        sit_set_led(2, 0);
    } else {
        sit_set_led(1, 0);
    }

    uint32_t regStatus = sit_get_device_status();
    LOG_INF("statusreg = 0x%08x",regStatus);
    k_sleep(K_SECONDS(2)); // Allow Logging to write out put 

    int frame_sequenz = 0;
    k_yield();

	while (1) {
		regStatus = sit_get_device_status();
		LOG_INF("sequence(%u) starting ; statusreg = 0x%08x",frame_sequenz,regStatus);
		sit_receiveNow(0);
        msg_header_t rx_poll_msg;
		msg_id_t msg_id = twr_1_poll;
		if(sit_checkReceivedIdMsg(msg_id, &rx_poll_msg)){
			uint64_t poll_rx_ts = get_rx_timestamp_u64();
            
            uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

            uint32_t resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
            
            msg_ss_twr_final_t msg_ss_twr_final_t = {
                    ss_twr_2_resp, (uint8_t)(rx_poll_msg.header.sequence + 1),
                    rx_poll_msg.header.dest , rx_poll_msg.header.source,(uint32_t)poll_rx_ts, resp_tx_ts,0
                };
            sit_sendAt((uint8_t*)&msg_ss_twr_final_t, sizeof(msg_ss_twr_final_t), resp_tx_time);

		} else {
			LOG_WRN("Something is wrong");
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		}
		regStatus = sit_get_device_status();
		LOG_INF("sequence(%u) ending ; statusreg = 0x%08x",frame_sequenz,regStatus);
		frame_sequenz++;
        k_sleep(K_MSEC(RNG_DELAY_MS));
	}

    return 0;
}
