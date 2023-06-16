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

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <sit.h>
#include <sit_led.h>
#include <ble_init.h>
#include <ble_device.h>

#define APP_NAME "SIMPLE TWR Initiator BLE EXAMPLE\n"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

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

/* Inter-ranging delay period, in milliseconds. */
#define RNG_DELAY_MS 100

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Indexes to access some of the fields in the frames defined above. */
#define ALL_MSG_SN_IDX          2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN         4
/* Frame sequence number, incremented after each transmission. */
static uint8_t frame_seq_nb = 0;

/* Delay between frames, in UWB microseconds. See NOTE 1 below. */
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
/* Receive response timeout. See NOTE 5 below. */
#define RESP_RX_TIMEOUT_UUS 400

uint32_t regStatus = 0;

int main(void) {
	printk(APP_NAME);
	printk("==================\n");

	int init_ok = sit_init(&config, TX_ANT_DLY, RX_ANT_DLY);
	// INIT LED and let them Blink one Time to see Intitalion Finshed
    sit_led_init();
    ble_device_init();

    if(init_ok < 0){
        sit_set_led(2, 0);
    } else {
        sit_set_led(1, 0);
    }
    
    regStatus = sit_get_device_status();
    LOG_INF("statusreg = 0x%08x",regStatus);
    k_sleep(K_SECONDS(2)); // Allow Logging to write out put 

    uint8_t frame_sequenz = 0;
    k_yield();

    while (1) {
        k_sleep(K_SECONDS(1));
        if (is_connected()) {
            sit_set_led(3, 0);
        } else {
            sit_toggle_led(3);
        }
        while (is_connected() && (ble_get_command() == 5)) {
            regStatus = sit_get_device_status();
            LOG_INF("initiator> sequence(%u) starting ; statusreg = 0x%08x",frame_sequenz,regStatus);
            sit_setRxAfterTxDelay(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
            msg_header_t twr_poll = {twr_1_poll, frame_sequenz, initiator_node_id , responder_node_id, 0};
            sit_startPoll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));
            regStatus = sit_get_device_status();
            LOG_INF("statusreg = 0x%08x",regStatus);

            frame_sequenz++;

            msg_ss_twr_final_t rx_final_msg;
            msg_id_t msg_id = ss_twr_2_resp;
            regStatus = sit_get_device_status();
            if(sit_checkReceivedIdFinalMsg(msg_id, rx_final_msg)) {
                uint64_t poll_tx_ts = get_tx_timestamp_u64();
                uint64_t resp_rx_ts = get_rx_timestamp_u64();
                
                uint64_t poll_rx_ts = rx_final_msg.poll_rx_ts;
                uint64_t resp_tx_ts = rx_final_msg.resp_tx_ts;
    
                float clockOffsetRatio = dwt_readcarrierintegrator() * 
                        (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6) ;

                uint64_t rtd_init = resp_rx_ts - poll_tx_ts;
                uint64_t rtd_resp = resp_tx_ts - poll_rx_ts;

                float tof =  ((rtd_init - rtd_resp * 
                        (1 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;
                
                float distance = tof * SPEED_OF_LIGHT;
                printk("initiator -> responder Distance: %3.2lf \n", distance);
                if((int) distance >= 0) {
                    ble_sit_notify(distance);
                }
            } else {
                LOG_WRN("Something is wrong");
                dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            }
            k_sleep(K_MSEC(RNG_DELAY_MS));
        }
	}
    return 0;
}
