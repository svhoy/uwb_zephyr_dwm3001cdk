#include <stdio.h>

#include <kernel.h>
#include <sys/printk.h>
#include <logging/log.h>

#include "sit_led.h"

LOG_MODULE_REGISTER(SIT_LED, LOG_LEVEL_WRN);

int led0_state = 0;
int led1_state = 0;
int led2_state = 0;
int led3_state = 0;

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);


void sit_led_init(void) {
  
	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set_dt(&led0, 0);
	gpio_pin_set_dt(&led1, 0);
	gpio_pin_set_dt(&led2, 0);
	gpio_pin_set_dt(&led3, 0);
	k_sleep(K_MSEC(1000));
	gpio_pin_set_dt(&led0, 1);
	gpio_pin_set_dt(&led1, 1);
	gpio_pin_set_dt(&led2, 1);
	gpio_pin_set_dt(&led3, 1);
	k_sleep(K_MSEC(1000));
    gpio_pin_set_dt(&led0, 0);
	gpio_pin_set_dt(&led1, 0);
	gpio_pin_set_dt(&led2, 0);
	gpio_pin_set_dt(&led3, 0);
    
}

int toggle_led_state (int state) {
    return 1 - state;
}

void sit_toggle_led(int led_id) {
    switch (led_id)
    {
    case 0:
        led0_state = toggle_led_state (led0_state);
        gpio_pin_set_dt(&led0, led0_state);
        break;
    case 1:
        led1_state = toggle_led_state (led1_state);
        gpio_pin_set_dt(&led1, led1_state);
        break;
    case 2:
        led2_state = toggle_led_state (led2_state);
        gpio_pin_set_dt(&led2, led2_state);
        break;
    case 3:
        led3_state = toggle_led_state (led3_state);
        gpio_pin_set_dt(&led3, led3_state);
        break;
    
    default:
        break;
    }
}

void sit_set_led(int led_id, int state) {
    switch (led_id)
    {
    case 0:
        led0_state = state;
        gpio_pin_set_dt(&led0, led0_state);
        break;
    case 1:
        led1_state = state;
        gpio_pin_set_dt(&led1, led1_state);
        break;
    case 2:
        led2_state = state;
        gpio_pin_set_dt(&led2, led2_state);
        break;
    case 3:
        led3_state = state;
        gpio_pin_set_dt(&led3, led3_state);
        break;
    
    default:
        break;
    }

}
