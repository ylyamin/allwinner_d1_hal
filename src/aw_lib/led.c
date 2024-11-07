#include "platform.h"

#include "gpio.h"

#include "led.h"

struct gpio_t led_gpio[] = {
	{
		.gpio = GPIOC,
		.pin = BV(1),
		.mode = GPIO_MODE_OUTPUT,
		.drv = GPIO_DRV_1,
	},
};

void led_init(void)
{
	gpio_init(led_gpio, ARRAY_SIZE(led_gpio));
}

void led_set(uint32_t idx, uint32_t state)
{
	if (idx >= ARRAY_SIZE(led_gpio))
		return;


	gpio_set(&led_gpio[idx], state?GPIO_SET:GPIO_RESET);
}
