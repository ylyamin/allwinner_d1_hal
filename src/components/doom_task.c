#include <platform.h>
#include <log.h>
#include <doom1_wad.h>
#include <tlsf.h>
#include <string.h>
#include <stdio.h>
#include <ccu.h>
#include <de.h>
#include <console_task.h>

//#define DOOM_IMPLEMENTATION
#define DOOM_FAST_TICK  1
#include "DOOM.h"
#include "doomdef.h"
#include "doomtype.h"

extern unsigned char __HeapBase; 
extern unsigned char __HeapLimit; 
static tlsf_t mem_pool = NULL;
extern unsigned char fb1[]; 

void doom_print_fnc(const char* fmt)
{
    small_printf(fmt);
}

void* doom_malloc_fnc(int size) 
{
    return tlsf_malloc(mem_pool, size);
}
void doom_free_fnc(void* ptr) 
{
	tlsf_free(mem_pool, ptr);
}

//xxd -i doom1.wad > doom1_wad.h

int file_counter = 0;

void* doom_open_fnc(const char* filename, const char* mode)
{
    if(strcmp(filename,"/home/doom1.wad") == 0)
    {
        file_counter = 0;
        return &doom1_wad;
    }     
    else
        return 0;
}

void doom_close_fnc(void* handle) 
{
    LOG_W("doom_close_fnc");
}

int doom_read_fnc(void* handle, void *buf, int count)
{
    if(file_counter <= doom1_wad_len)
    {
        memcpy(buf, handle + file_counter, count);
        file_counter += count;
        return count;
    }
    else    
        return 0;
}

int doom_write_fnc(void* handle, const void *buf, int count)
{
    LOG_W("doom_write_fnc");
    return -1;
}
int doom_seek_fnc(void* handle, int offset, doom_seek_t origin)
{
    if(origin == DOOM_SEEK_SET) file_counter = offset;
    if(origin == DOOM_SEEK_CUR) file_counter += offset;
    if(origin == DOOM_SEEK_END) file_counter = doom1_wad_len + offset;
    return 0;
}
int doom_tell_fnc(void* handle)
{
    return file_counter;
}
int doom_eof_fnc(void* handle)
{
    if(file_counter == doom1_wad_len)
        return 1;
    else
        return 0;
}    

void doom_gettime_fnc(int* sec, int* usec)
{
    *sec =  (int)(get_time_ms() / 1000);
    *usec = (int) get_time_us();
}

char* doom_getenv_fnc(const char* var) 
{ 
    return "/home"; 
}

extern byte* screens[];

void doom_task_init(void)
{
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
    doom_set_gettime(doom_gettime_fnc);
    //doom_set_exit(doom_exit_fnc);
    doom_set_getenv(doom_getenv_fnc);

    char argv[0]; 
    doom_init(0, *argv, 0);
    de_layer_set(doom_get_framebuffer(4), 0);
}

extern struct fifo_t console_fifo;

extern void* doom_memcpy(void* destination, const void* source, int num);

void doom_task_exec(void)
{
    uint8_t ch;
    if(fifo_empty(&console_fifo) != 1)
    {
        ch = console_read();
        LOG_I("%c",ch);
        doom_key_down(ch);
        //doom_button_down(doom_button_t button);
    }

    doom_force_update();
    doom_get_framebuffer(4);

    doom_key_up(ch);
    //doom_button_up(doom_button_t button);
    //LOG_D("%d",get_time_us());
}


/* void doom_key_down(doom_key_t key);
void doom_key_up(doom_key_t key);
void doom_button_down(doom_button_t button);
void doom_button_up(doom_button_t button);
void doom_mouse_move(int delta_x, int delta_y);
 */