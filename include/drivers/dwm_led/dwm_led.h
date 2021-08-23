/*
 *  ble_device.h
 */
#ifndef __DWM_LED_H__
#define __DWM_LED_H__

#include <drivers/gpio.h>

#define LED0_NODE 	DT_ALIAS(led0)
#define LED0		DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_PIN	DT_GPIO_PIN(LED0_NODE, gpios)

#define LED1_NODE 	DT_ALIAS(led1)
#define LED1		DT_GPIO_LABEL(LED1_NODE, gpios)
#define LED1_PIN	DT_GPIO_PIN(LED1_NODE, gpios)

#define LED2_NODE 	DT_ALIAS(led2)
#define LED2		DT_GPIO_LABEL(LED2_NODE, gpios)
#define LED2_PIN	DT_GPIO_PIN(LED2_NODE, gpios)

#define LED3_NODE 	DT_ALIAS(led3)
#define LED3		DT_GPIO_LABEL(LED3_NODE, gpios)
#define LED3_PIN	DT_GPIO_PIN(LED3_NODE, gpios)


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void dwm_led_init(void);
void dwm_toggle_led(int led_id);
void dwm_set_led(int led_id, int state);
                   
#endif  // __DWM_LED_H__