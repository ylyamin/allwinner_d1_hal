/*
 * File: log.c
 * Author: ylyamin
 */
#include <log.h>
#include <console_task.h>
#include <tinyprintf.h>

extern putcf stdout_putf;
extern void *stdout_putp;

int small_printf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    tfp_format(stdout_putp, stdout_putf, fmt, va); //print to uart

    if(console_task_init_finish()) //print to dispaly
    {
        char str_out[200];
        int retval = tfp_vsnprintf(str_out, 200, fmt, va);

        for(int j = 0; j < retval; j++)
        {
            console_string_buf_write(str_out[j]);
        }

    }
    va_end(va);
}