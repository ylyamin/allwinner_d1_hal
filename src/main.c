#include <main.h>
#include <ccu.h>
#include <uart.h>
#include <led.h>

void main(void)
{
	//csr_write_mie(1);
	ccu_init();

	uart_init(115200);
	small_printf("\n\nHello from allwinner\n\r");

	task_usb();

	led_init();
	led_set(0, 1);
}

void handle_trap(void)
{
	small_printf("Handle_trap\n\r");
	while(1)
	{

	}
}