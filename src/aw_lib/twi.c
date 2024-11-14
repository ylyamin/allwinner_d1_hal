#include "platform.h"
#include "gpio.h"
#include "ccu.h"
#include "log.h"
#include "twi.h"

struct gpio_t twi_gpio[] = {
	{ // sck
		.gpio = GPIOB,
		.pin = BV(10),
		.mode = GPIO_MODE_FNC2,
		.drv = GPIO_DRV_3,
	},
	{ // sda
		.gpio = GPIOB,
		.pin = BV(11),
		.mode = GPIO_MODE_FNC2,
		.drv = GPIO_DRV_3,
	},
};

void twi_set_clock(TWI_TypeDef *twi, uint32_t tgtfreq)
{
	uint32_t freq;
	uint32_t delta;
	uint32_t apb1 = ccu_apb1_clk_get();

	uint32_t best = 0xffffffff, best_n = 7, best_m = 15;

	for (int n = 0; n <= 7; n++) {
		for (int m = 0; m <= 15; m++) {
			freq = apb1 / (10 * (m+1) * (1 << n));
			delta = tgtfreq - freq;
			if (delta < best) {
				best = delta;
				best_n = n;
				best_m = m;
			}
			if (delta == 0) {
				break;
			}
		}
	}
	LOG_D("twi: clk %d n=%d m=%d\n\r", best, best_n, best_m);
	twi->TWI_CCR = (best_m << 3) | best_n;
}

void twi_init(void)
{
	LOG_D("twi: init\n\r");
	gpio_init(twi_gpio, ARRAY_SIZE(twi_gpio));
	ccu_twi_enable(TWI0);
	twi_set_clock(TWI0, 100000);
}
