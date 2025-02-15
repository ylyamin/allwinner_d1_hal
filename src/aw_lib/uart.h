#ifndef _UART_h_
#define _UART_h_
#include "platform.h"
#include <stdint.h>
#include <stddef.h>

void uart_init(UART_TypeDef *uart, uint32_t baudrate);
void uart_init_dma(void);
void uart_send(const char *buf, size_t count);
void uart_putchar(char c);
void _uart_putchar(char c);
void uart_putc( void* p, char c);
void uart_printf(const char *fmt, ...);
void uart_dump_reg(uint32_t base, size_t count);

#endif
