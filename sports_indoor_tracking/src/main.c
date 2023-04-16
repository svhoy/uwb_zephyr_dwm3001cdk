/**
 *
 * Copyright (c) 2022 - Sven Hoyer
 * 
 * Sports Indoor Tracking System Main
 * 
 */
#include <deca_device_api.h>

#include <sit.h>
#include <sit_led.h>
#include <sit_json.h>

#include <ble_init.h>
#include <ble_device.h>

#define APP_NAME "Sports Indoor Tracking\n"

#include <zephyr/kernel.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sit_main, LOG_LEVEL_INF);

#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

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

void main(void) {
	printk(APP_NAME);
	printk("==================\n");

	sit_led_init();
	if (sit_ble_init()) {
		LOG_ERR("Bluetooth init failed");
	}
	

	uint8_t init_error = 0;

	// repeat configuration when failed
	do {
		init_error = sit_init(&config, TX_ANT_DLY, RX_ANT_DLY);
	} while (init_error > 1);
	
	LOG_INF("Init Fertig ");
	
	uint32_t dev_id = dwt_getpartid();
	
	LOG_INF("Device ID: %lu", dev_id);

	
	// while(42) { //Life, the universe, and everything

	// }

}
