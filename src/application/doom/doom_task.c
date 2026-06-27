#include <platform.h>
#include <log.h>
#include <doom1_wad.h>
#include <tlsf.h>
#include <string.h>
#include <stdio.h>
#include <ccu.h>
#include <de.h>
#include <console_task.h>
#include <hid_app.h>

#include "DOOM.h"
#include "doomdef.h"
#include "doomtype.h"
#include <g2d.h>

extern tlsf_t mem_pool;
extern struct fifo_t keyboard_fifo;
extern struct fifo_t joystick_fifo;

extern uint32_t *doom_framebuffer;
extern uint32_t *framebuffer;

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

void doom_task_init(void)
{
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
    //de_layer_set(framebuffer2, 0);
}

uint8_t key, previous_key, frame_cnt;

void doom_task_exec(void)
{

    if(fifo_empty(&keyboard_fifo) != 1)
    {
        key = keyboard_read();
        if(key == HID_KEY_ENTER          )key = DOOM_KEY_ENTER;
        if(key == HID_KEY_SPACE          )key = DOOM_KEY_SPACE;
        if(key == HID_KEY_ARROW_RIGHT    )key = DOOM_KEY_RIGHT_ARROW;
        if(key == HID_KEY_ARROW_LEFT     )key = DOOM_KEY_LEFT_ARROW;
        if(key == HID_KEY_ARROW_DOWN     )key = DOOM_KEY_DOWN_ARROW;
        if(key == HID_KEY_ARROW_UP       )key = DOOM_KEY_UP_ARROW;
        if(key == HID_KEY_CONTROL_RIGHT  )key = DOOM_KEY_CTRL;
        if(key == HID_KEY_CONTROL_LEFT   )key = DOOM_KEY_CTRL;
        doom_key_down(key);
    }

    if(fifo_empty(&joystick_fifo) != 1)
    {
        joystick_out_t joystick_out = (joystick_out_t) joystick_read();
        ( joystick_out.bit.button_b) ? doom_key_down(DOOM_KEY_ENTER      ) : doom_key_up(DOOM_KEY_ENTER      );
        ( joystick_out.bit.button_y) ? doom_key_down(DOOM_KEY_SPACE      ) : doom_key_up(DOOM_KEY_SPACE      );
        ( joystick_out.bit.button_a) ? doom_key_down(DOOM_KEY_CTRL       ) : doom_key_up(DOOM_KEY_CTRL       );
        ( joystick_out.bit.x_left  ) ? doom_key_down(DOOM_KEY_LEFT_ARROW ) : doom_key_up(DOOM_KEY_LEFT_ARROW );
        ( joystick_out.bit.x_right ) ? doom_key_down(DOOM_KEY_RIGHT_ARROW) : doom_key_up(DOOM_KEY_RIGHT_ARROW);
        ( joystick_out.bit.y_up  )   ? doom_key_down(DOOM_KEY_UP_ARROW   ) : doom_key_up(DOOM_KEY_UP_ARROW   );
        ( joystick_out.bit.y_down )  ? doom_key_down(DOOM_KEY_DOWN_ARROW ) : doom_key_up(DOOM_KEY_DOWN_ARROW );
    }

        uint64_t frame_start = get_time_ms();

    doom_force_update();
   	g2d_rot(doom_framebuffer, framebuffer);

        uint64_t frame_stop = get_time_ms();

    if (previous_key && (++frame_cnt == 2))
    {
        doom_key_up(previous_key);
        frame_cnt = 0;
    }
    previous_key = key;

    //doom_button_up(doom_button_t button);
    //doom_mouse_move(int delta_x, int delta_y);
    
    //LOG_D("FPS: %d", 1000 / (frame_stop - frame_start));
}