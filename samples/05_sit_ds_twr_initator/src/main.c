/*! ----------------------------------------------------------------------------
 *  @file    ss_twr_initiator.c
 *  @brief   Single-sided two-way ranging (SS TWR) initiator example code
 *
 *           This is a simple code example which acts as the initiator in a SS TWR distance measurement exchange. This application sends a "poll"
 *           frame (recording the TX time-stamp of the poll), after which it waits for a "response" message from the "DS TWR responder" example
 *           code (companion to this application) to complete the exchange. The response message contains the remote responder's time-stamps of poll
 *           RX, and response TX. With this data and the local time-stamps, (of poll TX and response RX), this example application works out a value
 *           for the time-of-flight over-the-air and, thus, the estimated distance between the two devices, which it writes to the LCD.
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
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

/* Example application name */
#define APP_NAME "SS TWR INIT v1.0"

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
#define RNG_DELAY_MS 1000

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

    if(init_ok < 0){
        sit_set_led(2, 0);
    } else {
        sit_set_led(1, 0);
    }
    
    uint32_t regStatus = sit_get_device_status();
    LOG_INF("statusreg = 0x%08x",regStatus);
    k_sleep(K_SECONDS(2)); // Allow Logging to write out put 

    uint8_t frame_sequenz = 0;
    k_yield();
    
	while (1) {
        regStatus = sit_get_device_status();
        LOG_INF("initiator> sequence(%u) starting ; statusreg = 0x%08x",frame_sequenz,regStatus);
        sit_setRxAfterTxDelay(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
        msg_header_t twr_poll = {twr_1_poll, frame_sequenz, this_initiator_node_id , responder_node_id,0};
        sit_startPoll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));
        regStatus = sit_get_device_status();
        LOG_INF("statusreg = 0x%08x",regStatus);

        frame_sequenz++;

        msg_ss_twr_final_t rx_final_msg;
		msg_id_t msg_id = ss_twr_2_resp;
        regStatus = sit_get_device_status();
        if(sit_checkReceivedIdFinalMsg(msg_id, &rx_final_msg)) {
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
		} else {
			LOG_WRN("Something is wrong");
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		}
        k_sleep(K_MSEC(RNG_DELAY_MS));
	}
    return 0;
}