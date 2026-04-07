#include <platform.h>
#include <log.h>
#include <doom1_wad.h>
#include <tlsf.h>
#include <string.h>

//#define DOOM_IMPLEMENTATION
#include "DOOM.h"

static tlsf_t mem_pool = NULL;
extern uint8_t fb1[]; 

void doom_print_fnc(const char* fmt)
{
    LOG_D("%s",fmt);
}

void* doom_malloc_fnc(int size) 
{
    return tlsf_malloc(mem_pool, size);
}
void doom_free_fnc(void* ptr) 
{
	tlsf_free(mem_pool, ptr);
}

void* doom_open_fnc(const char* filename, const char* mode)
{
    LOG_W("%s",filename);
    if(strcmp(filename,"/home/doom1.wad") == 0)
        return 1;
    else
        return 0;
}

void doom_close_fnc(void* handle) 
{
    LOG_W("doom_close_fnc");
}


int doom_read_fnc(void* handle, void *buf, int count)
{
    LOG_W("doom_read_fnc");

    src_lib_puredoom_doom1_wad_len -= count;
    memcpy(buf, src_lib_puredoom_doom1_wad, count);

    if (src_lib_puredoom_doom1_wad_len)
        return 1;
    else     
        return -1;
}

int doom_write_fnc(void* handle, const void *buf, int count)
{
    LOG_W("doom_write_fnc");
    return -1;
}
int doom_seek_fnc(void* handle, int offset, doom_seek_t origin)
{
    LOG_W("doom_seek_fnc");
    return -1;
}
int doom_tell_fnc(void* handle)
{
    LOG_W("doom_tell_fnc");
    return -1;
}
int doom_eof_fnc(void* handle)
{
    LOG_W("doom_eof_fnc");
    return 1;
}

char* doom_getenv_fnc(const char* var) 
{ 
    return "/home"; 
}


void doom_task_init(void)
{
    extern unsigned char __HeapBase; 
    extern unsigned char __HeapLimit; 
    size_t s = (size_t)(&__HeapLimit - &__HeapBase);
	LOG_W("heap: creating mem pool @ %08x size %d\n", &__HeapBase, s);
	mem_pool = tlsf_create_with_pool((void *)&__HeapBase, s);

    doom_set_print(doom_print_fnc);
    doom_set_malloc(doom_malloc_fnc, doom_free_fnc);
    doom_set_file_io( doom_open_fnc,
                      doom_close_fnc,
                      doom_read_fnc,
                      doom_write_fnc,
                      doom_seek_fnc,
                      doom_tell_fnc,
                      doom_eof_fnc);
    doom_set_getenv(doom_getenv_fnc);



    char argv[0]; 
    doom_init(0, *argv, 0);
}


void doom_task_exec(void)
{
    doom_update();
    *fb1 = doom_get_framebuffer(4 /* RGBA */);
}


//xxd -i src/lib/puredoom/doom1.wad > doom1.wad.h
