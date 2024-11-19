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

#include <gr.h>
#include <de.h>
#include <tcon_lcd.h>
#include <icn9707_480x1280.h>



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

/*
axp_USB_control(TWI0,1);
task_usb(); 
*/
	
////////////////////////////////////////////////////////////
	led_init();
	LOG_I("led init");

	uint32_t h = de_layer_get_h();
	uint32_t w = de_layer_get_w();

	uint32_t size = h * w * 4;
	uint8_t fb1[size];  //= dma_memalign(128, size);
	uint8_t fb2[size]; // = dma_memalign(128, size);

	LOG_D("fb addr: %08x and %08x\n", &fb1, &fb2);

	gr_fill(&fb1, 0xff000000);
	gr_fill(&fb2, 0xff000000);
	gr_draw_pixel(&fb1, 100, 100, 0xffff0000);
	gr_draw_pixel(&fb1, 101, 101, 0xffff0000);
	gr_draw_pixel(&fb1, 101, 100, 0xffff0000);
	gr_draw_pixel(&fb1, 100, 101, 0xffff0000);
	gr_draw_line(&fb1, 0, 0, w-1, h-1, 0xff00ff00);
	gr_draw_line(&fb1, w-1, 0, 0, h-1, 0xffff0000);

	tcon_lcd_init();
	tcon_lcd_enable();
	led_set(0, 1);

	LCD_panel_init();

	de_init();
	de_layer_set(&fb1, &fb2);

//////////////////////////////////////////////////////////////


}