//
// first uart is initialized by DRAM init code, it is 115200, uart_dma_mode == 0 (also after calling uart_init)
// 
// when uart_dma_mode == 0:
// - uart uses blocking access, one by one byte is transmitted when uart is not busy
//
// when uart_dma_mode == 1:
// - texts to print are stored in output fifo and transmission is done by DMA requests
// - access to send function is locked
//
// uart_init_dma needs to be called with FreeRTOS scheduller running (or between uart_init_dma
// and scheduler start, there can be no uart output) otherwise you get freeze.
//
#include "platform.h"
#include <config.h>

#include <stdarg.h>
#include <stdio.h>

#include "gpio.h"
#include "ccu.h"
//#include "dmac.h"
//#include "fifo.h"
//#include "mmu.h"

#include "uart.h"

//#include "FreeRTOS.h"
//#include "semphr.h"

#define UART_DMAC_CH 0
#define UART_TX_BUF 4096

//struct fifo_t uart_tx_fifo;

// both these are placed in non cached section
//uint8_t __attribute__((section(".uart_ram"))) uart_tx_buf[UART_TX_BUF];
//struct __attribute__((section(".uart_ram"), aligned(16))) dmac_desc_t uart_dmac_desc;

static uint32_t uart_sending_size;
static volatile int uart_sending;

static int uart_dma_mode = 0;
//SemaphoreHandle_t uart_mutex;

struct gpio_t uart_gpio[] = {
	{
		.gpio = CONFIG_UART_PIN_GROUP,
		.pin = BV(CONFIG_UART_PIN_TX),
		.mode = GPIO_MODE_FNC6,
		.drv = GPIO_DRV_0,
	},
	{
		.gpio = CONFIG_UART_PIN_GROUP,
		.pin = BV(CONFIG_UART_PIN_RX),
		.mode = GPIO_MODE_FNC6,
		.drv = GPIO_DRV_0,
	},
};

void uart_init(UART_TypeDef *uart, uint32_t baudrate)
{
	gpio_init(uart_gpio, ARRAY_SIZE(uart_gpio));

	ccu_uart_enable(uart);

	// calculare baudrate divisor
	uint32_t clk = ccu_apb1_clk_get();

	uint32_t br = clk/16/baudrate;

	uart->UART_DLH_IER = 0x0;
	uart->UART_IIR_FCR = 0xf7;
	uart->UART_MCR = 0x0;

	// enable access to DLL DLH
	uart->UART_LCR |= (1 << 7);

	// configure baudrate prescaler
	uart->UART_RBR_THR_DLL = br & 0xff;
	uart->UART_DLH_IER = (br >> 8) & 0xff;

	// disable access to DLL DLH
	uart->UART_LCR &= ~(1 << 7);

	// configure line config 8,n,1
	uart->UART_LCR &= ~0x1f;
	uart->UART_LCR |= (0x3 << 0) | (0 << 2) | (0x0 << 3);
}

UART_TypeDef *uart = CONFIG_UART_NUM;

void _uart_putchar(char c)
{
	while ((uart->UART_USR & 2) == 0);
	uart->UART_RBR_THR_DLL = c;
	while ((uart->UART_USR & 2) == 0);
}

void uart_putchar(char c)
{
	_uart_putchar(c);
	if (c == '\n') {
			_uart_putchar('\r');
	}
}

void uart_putc ( void* p, char c)
		{
			uart_putchar(c);
		}