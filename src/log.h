/*
 * File: log.h
 * Author: ylyamin
 */
#ifndef LOG_H
#define LOG_H

#include <config.h>
#include <tinyprintf.h>

#define small_printf tfp_printf

/* DEBUG level */
#define LOG_ERROR           0
#define LOG_WARNING         1
#define LOG_INFO            2
#define LOG_DBG             3

#ifndef LOG_LEVEL
#define LOG_LEVEL         LOG_ERROR
#endif

#define LOG_COLOR_OFF      "\033[0m"
#define LOG_COLOR_RED      "\033[1;40;31m"
#define LOG_COLOR_YELLOW   "\033[1;40;33m"
#define LOG_COLOR_BLUE     "\033[1;40;34m"
#define LOG_COLOR_PURPLE   "\033[1;40;35m"

#define LOG_ERROR_PREFIX   "[ERR]"
#define LOG_WARNING_PREFIX "[WRN]"
#define LOG_INFO_PREFIX    "[INF]"
#define LOG_DEBUG_PREFIX   "[DBG]"

#define LOG_LAYOUT      "%s%s%s:%s"
#define LOG_COLOR(log_lv, log_color, log_format, ...)   \
    small_printf(LOG_LAYOUT log_format "%s""\n\r",      \
           log_color, log_lv, LOG_COLOR_OFF,            \
           log_color, ##__VA_ARGS__, LOG_COLOR_OFF)

#if (LOG_LEVEL >= LOG_DBG)
#define LOG_D(...) do { LOG_COLOR(LOG_DEBUG_PREFIX, LOG_COLOR_PURPLE, ##__VA_ARGS__); } while(0)
#else
#define LOG_D(...)
#endif

#if (LOG_LEVEL >= LOG_INFO)
#define LOG_I(...) do { LOG_COLOR(LOG_INFO_PREFIX, LOG_COLOR_BLUE, ##__VA_ARGS__); } while(0)
#else
#define LOG_I(...)
#endif

#if (LOG_LEVEL >= LOG_WARNING)
#define LOG_W(...) do { LOG_COLOR(LOG_WARNING_PREFIX, LOG_COLOR_YELLOW, ##__VA_ARGS__); } while(0)
#else
#define LOG_W(...)
#endif

#if (LOG_LEVEL >= LOG_ERROR)
#define LOG_E(...) do { LOG_COLOR(LOG_ERROR_PREFIX, LOG_COLOR_RED, ##__VA_ARGS__); } while(0)
#else
#define LOG_E(...)
#endif

#endif /*LOG_H*/