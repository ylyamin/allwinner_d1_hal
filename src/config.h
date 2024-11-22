/*
 * File: config.c
 * Author: ylyamin
 */
#ifndef CONFIG_H_
#define CONFIG_H_
#include <platform.h>

#define CONFIG_LOG_LEVEL    LOG_DBG

#if (SOC == SOC_D1H)

#define CONFIG_UART_NUM         UART0
#define CONFIG_UART_BAUDRATE    115200
#define CONFIG_UART_PIN_GROUP   GPIOB
#define CONFIG_UART_PIN_TX      8
#define CONFIG_UART_PIN_RX      9

#elif (SOC == SOC_D1S)

#define CONFIG_UART_NUM         UART0
#define CONFIG_UART_BAUDRATE    500000
#define CONFIG_UART_PIN_GROUP   GPIOE
#define CONFIG_UART_PIN_TX      2
#define CONFIG_UART_PIN_RX      3

#endif

//#define CONFIG_USE_DCACHE 1
//#define CONFIG_USE_PMP    1
//#define CONFIG_USE_MMU    1
#define CONFIG_USE_TWI      1
#define CONFIG_USE_USB      1
#define CONFIG_USE_DISPLAY  1

#endif  //CONFIG_H_