/*
 * File: log.h
 * Author: ylyamin
 */
#include <log.h>
#include <console_task.h>
#include <tinyprintf.h>

extern uint8_t console_task_init_done;

extern putcf stdout_putf;
extern void *stdout_putp;

int small_printf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    
    tfp_format(stdout_putp, stdout_putf, fmt, va);

    if(console_task_init_done)
    {
        char str_out[200];
        int retval = tfp_vsprintf(str_out, fmt, va);
        for(int j = 0; j < retval; j++)
        {
            console_string_buf_write(str_out[j]);
        }
    }
    va_end(va);
}