#include <zephyr.h>
#include <logging/log.h>
#include <stdio.h>
#include <drivers/gpio.h>

#include "drivers/sit_led/sit_led.h"

int led0_state = 0;
int led1_state = 0;
int led2_state = 0;
int led3_state = 0;

void dwm_led_init(void) {
    const struct device *dev = device_get_binding(LED0);

	gpio_pin_configure(dev, LED0_PIN, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure(dev, LED1_PIN, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure(dev, LED2_PIN, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure(dev, LED3_PIN, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set(dev, LED0_PIN, 0);
	gpio_pin_set(dev, LED1_PIN, 0);
	gpio_pin_set(dev, LED2_PIN, 0);
	gpio_pin_set(dev, LED3_PIN, 0);
	k_sleep(K_MSEC(1000));
	gpio_pin_set(dev, LED0_PIN, 1);
	gpio_pin_set(dev, LED1_PIN, 1);
	gpio_pin_set(dev, LED2_PIN, 1);
	gpio_pin_set(dev, LED3_PIN, 1);
	k_sleep(K_MSEC(1000));
}

int toggle_led_state (int state) {
    return 1 - state;
}

void dwm_toggle_led(int led_id) {
    const struct device *dev = device_get_binding(LED0);

    switch (led_id)
    {
    case 0:
        led0_state = toggle_led_state (led0_state);
        gpio_pin_set(dev, LED0_PIN, led0_state);
        break;
    case 1:
        led1_state = toggle_led_state (led1_state);
        gpio_pin_set(dev, LED1_PIN, led1_state);
        break;
    case 2:
        led2_state = toggle_led_state (led2_state);
        gpio_pin_set(dev, LED2_PIN, led2_state);
        break;
    case 3:
        led3_state = toggle_led_state (led3_state);
        gpio_pin_set(dev, LED3_PIN, led3_state);
        break;
    
    default:
        break;
    }
}

void dwm_set_led(int led_id, int state) {
       const struct device *dev = device_get_binding(LED0);

    switch (led_id)
    {
    case 0:
        led0_state = state;
        gpio_pin_set(dev, LED0_PIN, led0_state);
        break;
    case 1:
        led1_state = state;
        gpio_pin_set(dev, LED1_PIN, led1_state);
        break;
    case 2:
        led2_state = state;
        gpio_pin_set(dev, LED2_PIN, led2_state);
        break;
    case 3:
        led3_state = state;
        gpio_pin_set(dev, LED3_PIN, led3_state);
        break;
    
    default:
        break;
    }

}
