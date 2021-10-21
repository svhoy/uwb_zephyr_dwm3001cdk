/* 
*	main.c - 
*	Application main entry point 
*/
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include "drivers/sit_led/sit_led.h"
#include "drivers/sit.h"

#define APP_NAME "SIMPLE TWR Responder EXAMPLE\n"

#define LOG_LEVEL 3
#include <logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_ERR);

/* Inter-ranging delay period, in milliseconds. */
#define RNG_DELAY_MS 500

/* Default communication configuration. */
static dwt_config_t config = {
    5,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,      /* Data rate. */
    DWT_PHRMODE_EXT, /* PHY header mode. */
    (129)            /* SFD timeout (preamble length + 1 + SFD length - PAC size). 
                        Used in RX only. */           
};

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) 
 * conversion factor.
 * 1 uus = 512 / 499.2 usec and 1 usec = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536

/*rx twr_2_resp after tx twr_1_poll
 protected by responder's mp_request_at(twr_2_resp):POLL_RX_TO_RESP_TX_DLY_UUS
*/
#define POLL_TX_TO_RESP_RX_DLY_UUS 300

#define RESP_RX_TO_FINAL_TX_DLY_UUS 1000

#define STACKSIZE 2048


void main(void) {
	printk(APP_NAME);
	printk("==================\n");

	bool init_ok = sit_init(config);
	// INIT LED and let them Blink one Time to see Intitalion Finshed
    dwm_led_init();
    
    if(init_ok == false){
        dwm_set_led(2, 0);
    } else {
        dwm_set_led(1, 0);
    }

    uint32_t regStatus = sit_getRegStatus();
    LOG_INF("statusreg = 0x%08x",regStatus);
    k_sleep(K_SECONDS(2)); // Allow Logging to write out put 

    int frame_sequenz = 0;
    k_yield();

	while (1) {
		regStatus = sit_getRegStatus();
		LOG_INF("sequence(%u) starting ; statusreg = 0x%08x",frame_sequenz,regStatus);
		sit_receiveNow(0);
        msg_header_t rx_poll_msg;
		msg_id_t msg_id = twr_1_poll;
		if(sit_checkReceivedIdMsg(msg_id, rx_poll_msg)){
			uint64_t poll_rx_ts = get_rx_timestamp_u64();
            
            uint32_t resp_tx_time = (poll_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

            uint32_t resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
            
            msg_ss_twr_final_t msg_ss_twr_final_t = {
                    ss_twr_2_resp, (uint8_t)(rx_poll_msg.header.sequence + 1),
                    rx_poll_msg.header.dest , rx_poll_msg.header.source,poll_rx_ts, resp_tx_ts,0
                };
            sit_sendAt((uint8_t*)&msg_ss_twr_final_t, sizeof(msg_ss_twr_final_t), resp_tx_time);

		} else {
			LOG_WRN("Something is wrong");
            dwt_rxreset();
		}
		regStatus = sit_getRegStatus();
		LOG_INF("sequence(%u) ending ; statusreg = 0x%08x",frame_sequenz,regStatus);
		frame_sequenz++;
        k_sleep(K_MSEC(RNG_DELAY_MS));
	}
}
