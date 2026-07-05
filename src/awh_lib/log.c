/*
 * File: log.h
 * Author: ylyamin
 */
#include <log.h>
#include <console_task.h>

extern struct fifo_t console_string_buf_fifo;

int small_printf(const char *fmt, ...)
{
    int ret = tfp_printf(fmt);

    if(console_string_buf_fifo.buffer)
    {
        char str_out[1000];
        int num = tfp_sprintf(str_out, fmt);
        
        for(int j = 0; j < num; j++)
        {
            console_string_buf_write(str_out[j]);
        }
    }

    return ret;
}


