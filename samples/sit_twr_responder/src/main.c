/* 
*	main.c - 
*	Application main entry point 
*/
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include "drivers/sit_led/sit_led.h"

#define APP_NAME "SIMPLE TWR Responder EXAMPLE\n"

void main(void) {
	printk(APP_NAME);
	printk("==================\n");

	dwm_led_init();


	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

	}
}
