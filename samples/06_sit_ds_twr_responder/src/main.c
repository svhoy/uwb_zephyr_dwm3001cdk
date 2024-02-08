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
#include "deca_device_api.h"

#include <sit/sit.h>
#include <sit/sit_device.h>
#include <sit/sit_distance.h>
#include <sit/sit_config.h>
#include <sit/sit_utils.h>
#include <sit_led/sit_led.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define APP_NAME "SIMPLE DS-TWR Responder EXAMPLE\n"

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


#define POLL_RX_TO_RESP_TX_DLY_UUS_T 900
#define RESP_TX_TO_FINAL_RX_DLY_UUS_T 600
#define FINAL_RX_TIMEOUT_T 1200
#define PRE_TIMEOUT 5

int main(void) {
	printk(APP_NAME);
	printk("==================\n");

    // INIT LED and let them Blink one Time to see Intitalion Finshed
    sit_led_init();

	int init_ok = 0;
    do {
		init_ok = sit_init();
	} while (init_ok > 1);

    if(init_ok < 0){
        sit_set_led(2, 1);
    } else {
        sit_set_led(1, 1);
    }

    uint8_t frame_sequenz = 0;

	while (1) {
		sit_receive_now();
        msg_simple_t rx_poll_msg;
		msg_id_t msg_id = twr_1_poll;
		if(sit_check_msg_id(msg_id, &rx_poll_msg)){
			uint64_t poll_rx_ts = get_rx_timestamp_u64();
            
            uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS_T * UUS_TO_DWT_TIME)) >> 8;
            
            msg_simple_t msg_ds_poll_resp = {
                    ds_twr_2_resp,
                    rx_poll_msg.header.sequence,
                    rx_poll_msg.header.dest,
                    rx_poll_msg.header.source,
                    0
                };
            sit_set_rx_after_tx_delay(RESP_TX_TO_FINAL_RX_DLY_UUS_T);
            sit_set_rx_timeout(FINAL_RX_TIMEOUT_T);
            sit_set_preamble_detection_timeout(PRE_TIMEOUT);
            bool ret = sit_send_at_with_response((uint8_t*)&msg_ds_poll_resp, sizeof(msg_simple_t), resp_tx_time);
            if (ret == false) {
                continue;
                LOG_WRN("Something is wrong with Sending Poll Resp Msg");
            }
            msg_ds_twr_final_t rx_ds_final_msg;
		    msg_id = ds_twr_3_final;
            if(sit_check_ds_final_msg_id(msg_id, &rx_ds_final_msg)){
                uint64_t resp_tx_ts = get_tx_timestamp_u64();
                uint64_t final_rx_ts = get_rx_timestamp_u64();

                uint32_t poll_tx_ts = rx_ds_final_msg.poll_tx_ts;
                uint32_t resp_rx_ts = rx_ds_final_msg.resp_rx_ts;
                uint32_t final_tx_ts = rx_ds_final_msg.final_tx_ts;

                uint32_t poll_rx_ts_32, resp_tx_ts_32, final_rx_ts_32;
                poll_rx_ts_32 = (uint32_t) poll_rx_ts;
                resp_tx_ts_32 = (uint32_t) resp_tx_ts;
                final_rx_ts_32 = (uint32_t) final_rx_ts;

                double Ra, Rb, Da, Db;
                int64_t tof_dtu;
                Ra = (double)(resp_rx_ts - poll_tx_ts);
                Rb = (double)(final_rx_ts_32 - resp_tx_ts_32);
                Da = (double)(final_tx_ts - resp_rx_ts);
                Db = (double)(resp_tx_ts_32 - poll_rx_ts_32);
                tof_dtu = (int64_t)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));

                double tof = tof_dtu * DWT_TIME_UNITS;
                double distance = tof * SPEED_OF_LIGHT;
                LOG_INF("Distance: %lf", distance);
            } else {
                LOG_WRN("Something is wrong with Final Msg Receive");
                dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            }

		} else {
			LOG_WRN("Something is wrong with Poll Msg Receive");
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		}
		frame_sequenz++;
        k_msleep(80);
	}

    return 0;
}
