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
#include <twi.h>

extern uint32_t __bss_start__;
extern uint32_t __bss_end__;

extern void main(void);

void init_bss(uint32_t start, uint32_t end)
{
    uint32_t *dst;
    dst = &start;
    while (dst < &end)
    {
        *dst++ = 0ul;
    }
}

void logo(void)
{
	char * soc = "NA";
	char * platform = "NA";
	char * lcd = "NA";

	if (SOC == SOC_D1H) soc = "D1H";
	if (SOC == SOC_D1S) soc = "D1s";
	if (PLATFORM == PLATFORM_SIPEED)  platform = "Sipeed Lichee RV";
	if (PLATFORM == PLATFORM_DEVTERM) platform = "ClockworkPi Devterm R-01";
	if (LCD == LCD_LVDS) lcd = "LVDS";
	if (LCD == LCD_MIPI) lcd = "MIPI DSI";
	if (LCD == LCD_RGB)  lcd = "RGB";

	LOG_I("	\\ | /");
	LOG_I("	- Allwinner D1 HAL [ver: %s]",VERSION_GIT);
	LOG_I("	/ | \\");
	LOG_I("	SoC: %s",soc);
	LOG_I("	Platform: %s",platform);
	LOG_I("	LCD: %s", lcd);
}

extern int console_task_init_done;

void board_start(void)
{
	//init_bss(__bss_start__ , __bss_end__);
	ccu_init();
	uart_init(CONFIG_UART_NUM, CONFIG_UART_BAUDRATE);
	init_printf(NULL,uart_putc);
	logo();
	irq_init();

#ifdef CONFIG_USE_PMP
	csr_write_pmpaddr0((0x40000000 >> 2) | 0b011111111111111111);  //1M
	csr_write_pmpaddr1((0x40100000 >> 2) | 0b0111111111111111111); //2M
	csr_write_pmpcfg0(0x00009f9f); // LOCK | NAPOT | X | W | R
#endif

#ifdef CONFIG_USE_MMU
	csr_write_satp(0x00000000);
#endif

#ifdef CONFIG_USE_DCACHE
	dcache_enable();
#endif

#ifdef CONFIG_USE_TWI
	twi_init(TWI0, 400000);   
#endif

	main();
}