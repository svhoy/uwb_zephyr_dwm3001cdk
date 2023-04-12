#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "sit_led.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);


void main(void)
{
	printk("Welcome to SIT LED Test Example\n");
	
	sit_led_init();

	while (1) {
		sit_toggle_led(1);
		k_sleep(K_MSEC(1000));
		sit_toggle_led(1);
		sit_toggle_led(0);
		k_sleep(K_MSEC(1000));
		sit_toggle_led(0);
		sit_toggle_led(2);
		k_sleep(K_MSEC(1000));
		sit_toggle_led(2);
		sit_toggle_led(3);
		k_sleep(K_MSEC(1000));
		sit_toggle_led(3);
		sit_toggle_led(2);
		k_sleep(K_MSEC(1000));
		sit_toggle_led(2);
		sit_toggle_led(0);
		k_sleep(K_MSEC(1000));
		sit_toggle_led(0);
	}
}
