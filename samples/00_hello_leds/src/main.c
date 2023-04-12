#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED0_NODE 	DT_ALIAS(led0)

#define LED1_NODE 	DT_ALIAS(led1)

#define LED2_NODE 	DT_ALIAS(led2)

#define LED3_NODE 	DT_ALIAS(led3)


static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

void main(void)
{
	printk("Welcome to Hello LED Test Example\n");
	LOG_INF("Hello world leds sample");

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

	int count = 0;
	while (1) {
		count++;
		LOG_INF("leds> hello %d",count);
		gpio_pin_set_dt(&led1, 0);
		k_sleep(K_MSEC(1000));
		gpio_pin_set_dt(&led1, 1);
		gpio_pin_set_dt(&led0, 0);
		k_sleep(K_MSEC(1000));
		gpio_pin_set_dt(&led0, 1);
		gpio_pin_set_dt(&led2, 0);
		k_sleep(K_MSEC(1000));
		gpio_pin_set_dt(&led2, 1);
		gpio_pin_set_dt(&led3, 0);
		k_sleep(K_MSEC(1000));
		gpio_pin_set_dt(&led3, 1);
	}
}
