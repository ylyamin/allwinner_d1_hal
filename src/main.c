#include <main.h>
#include <ccu.h>
#include <uart.h>
#include <led.h>
#include <usb_task.h>

void main(void)
{
	ccu_init();

	uart_init(115200);
	small_printf("\n\nHello from allwinner\n\r");

	irq_init();
	small_printf("irq_init\n\r");

	task_usb(0);

	led_init();
	led_set(0, 1);
}