/*
 *  ble_device.h
 */
#ifndef __SIT_LED_H__
#define __SIT_LED_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <zephyr/drivers/gpio.h>

#define LED0_NODE 	DT_ALIAS(led0)
#define LED1_NODE 	DT_ALIAS(led1)
#define LED2_NODE 	DT_ALIAS(led2)
#define LED3_NODE 	DT_ALIAS(led3)

void sit_led_init(void);
void sit_toggle_led(int led_id);
void sit_set_led(int led_id, int state);

#ifdef __cplusplus
}
#endif

#endif  // __SIT_LED_H__