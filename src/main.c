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
#include <twi.h>
#include <axp228.h>

extern unsigned int __bss_start__;
extern unsigned int __bss_end__;
extern unsigned int __bss2_start__;
extern unsigned int __bss2_end__;

void init_bss(int start, int end)
{
    unsigned int *dst;
    dst = &start;
    while (dst < &end)
    {
        *dst++ = 0ul;
    }
}


void main(void)
{
	init_bss(__bss_start__ , __bss_end__);
	init_bss(__bss2_start__ , __bss2_end__);

	ccu_init();
	uart_init(115200);
	init_printf(NULL,uart_putc);
	LOG_I("Hello from allwinner !");
	LOG_I("ccu_perip_x1_clk_get: %d",ccu_perip_x1_to_clk());

#ifdef USE_PMP
	csr_write_pmpaddr0((0x40000000 >> 2) | 0b011111111111111111);  //1M
	csr_write_pmpaddr1((0x40100000 >> 2) | 0b0111111111111111111); //2M
	csr_write_pmpcfg0(0x00009f9f); // LOCK | NAPOT | X | W | R
#endif
#ifdef USE_MMU
	csr_write_satp(0x00000000);
#endif

	irq_init();

#ifdef USE_DCACHE
	dcache_enable();
#endif

	twi_init(TWI0, 400000);
    axp_USB_control(TWI0,1);
	task_usb();
	
	led_init();
	led_set(0, 1);
}