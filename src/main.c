/*
 * File: main.c
 * Author: ylyamin
 */
#include <config.h>
#include <platform.h>
#include <log.h>
#include <ccu.h>
#include <uart.h>
#include <irq.h>
#include <led.h>
#include <usb_task.h>

void main(void)
{
	ccu_init();
	uart_init(115200);
	init_printf(NULL,uart_putc);
	LOG_I("Hello from allwinner !");

	csr_write_pmpaddr0(0xffffffffULL);
	csr_write_pmpcfg0(0x0FULL); // PMP_R | PMP_W | PMP_X | PMP_MATCH_TOR);
	csr_write_satp(0);

	irq_init();

#ifdef USE_DCACHE
	dcache_enable();
#endif
	task_usb();
	
	led_init();
	led_set(0, 1);
}