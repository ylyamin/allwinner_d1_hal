#include <platform.h>
#include <log.h>

#define DOOM_IMPLEMENTATION
#include "PureDOOM.h"

extern uint8_t fb1[]; 

void doom_print_fnc(const char* fmt)
{
    small_printf(fmt);
}

void* doom_open_fnc(const char* filename, const char* mode)
{
    small_printf(filename);
    return 1;
}

void doom_close_fnc(void* handle) 
{
    small_printf("doom_close_fnc");
}


int doom_read_fnc(void* handle, void *buf, int count)
{
    small_printf("doom_read_fnc");
    return -1;
}

int doom_write_fnc(void* handle, const void *buf, int count)
{
    small_printf("doom_write_fnc");
    return -1;
}
int doom_seek_fnc(void* handle, int offset, doom_seek_t origin)
{
    small_printf("doom_seek_fnc");
    return -1;
}
int doom_tell_fnc(void* handle)
{
    small_printf("doom_tell_fnc");
    return -1;
}
int doom_eof_fnc(void* handle)
{
    small_printf("doom_eof_fnc");
    return 1;
}

void doom_task_init(void)
{
    doom_set_print(doom_print_fnc);
    doom_set_file_io( doom_open_fnc,
                      doom_close_fnc,
                      doom_read_fnc,
                      doom_write_fnc,
                      doom_seek_fnc,
                      doom_tell_fnc,
                      doom_eof_fnc);
    char argv[0]; 
    doom_init(0, *argv, 0);
}


void doom_task_exec(void)
{
    doom_update();
    *fb1 = doom_get_framebuffer(4 /* RGBA */);
}



