/*
 *  ble_device.h
 */
#ifndef __DWM_LED_H__
#define __DWM_LED_H__

#include <zephyr/drivers/gpio.h>

#define LED0_NODE 	DT_ALIAS(led0)
#define LED1_NODE 	DT_ALIAS(led1)
#define LED2_NODE 	DT_ALIAS(led2)
#define LED3_NODE 	DT_ALIAS(led3)

void dwm_led_init(void);
void dwm_toggle_led(int led_id);
void dwm_set_led(int led_id, int state);
                   
#endif  // __DWM_LED_H__