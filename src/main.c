#include <main.h>
#include <stdio.h>
#include <ccu.h>
#include <uart.h>
#include <led.h>
#include <usb_task.h>

extern void uart_putc ( void* p, char c);


void main(void)
{
	ccu_init();

	uart_init(115200);

	init_printf(NULL,uart_putc);
	small_printf("\n\nHello from allwinner\n\r");


	irq_init();
	small_printf("irq_init\n\r");

	task_usb();

	led_init();
	led_set(0, 1);
}