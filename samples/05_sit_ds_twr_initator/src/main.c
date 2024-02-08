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

/* Example application name */
#define APP_NAME "SIMPLE DS-TWR Initiator EXAMPLE\n"

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

#define CPU_PROCESSING_TIME 400
#define POLL_TX_TO_RESP_RX_DLY_UUS_T (350 + CPU_PROCESSING_TIME)
#define RESP_RX_TO_FINAL_TX_DLY_UUS_T (350 + CPU_PROCESSING_TIME)
#define RESP_RX_TIMEOUT_UUS_T 1150
#define PRE_TIMEOUT 5


int main(void) {
	printk(APP_NAME);
	printk("==================\n");
    
    // INIT LED and let them Blink one Time to see Intitalion Finshed
    sit_led_init();

	int init_ok = sit_init();
    
    if(init_ok < 0){
        sit_set_led(2, 0);
    } else {
        sit_set_led(1, 0);
    }
    uint8_t frame_sequenz = 0;
	while (1) {
        
        sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS_T, RESP_RX_TIMEOUT_UUS_T);
        sit_set_preamble_detection_timeout(PRE_TIMEOUT);

        msg_simple_t twr_poll = {twr_1_poll, frame_sequenz, this_initiator_node_id , responder_node_id,0};
        sit_start_poll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));

        msg_simple_t rx_resp_msg;
        msg_id_t msg_id = ds_twr_2_resp;

        if(sit_check_msg_id(msg_id, &rx_resp_msg)) {
            uint64_t poll_tx_ts = get_tx_timestamp_u64();
			uint64_t resp_rx_ts = get_rx_timestamp_u64();
			
            uint32_t final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS_T * UUS_TO_DWT_TIME)) >> 8;
            uint64_t final_tx_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

            msg_ds_twr_final_t final_msg = {
                ds_twr_3_final,
                rx_resp_msg.header.sequence,
                rx_resp_msg.header.dest,
                rx_resp_msg.header.source,
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
            frame_sequenz++;

		} else {
			LOG_WRN("Something is wrong with Receiving Msg");
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		}
        k_msleep(100);
	}
    return 0;
}