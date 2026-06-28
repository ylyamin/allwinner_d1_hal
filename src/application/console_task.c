#include <platform.h>
#include <fifo.h>
#include <log.h>
#include <ccu.h>
#include <de.h>
#include <gr.h>
#include <font8x8_basic.h>
#include <font16x16.h>

#define BUFF_SIZE 100

struct fifo_t keyboard_fifo;
struct fifo_t mouse_fifo;
struct fifo_t joystick_fifo;

uint8_t keyboard_buf[BUFF_SIZE];
uint8_t mouse_buf[BUFF_SIZE];
uint8_t joystick_buf[BUFF_SIZE];

extern uint8_t *framebuffer; 

void console_task_init(void)
{
	fifo_init(&keyboard_fifo, keyboard_buf, sizeof(uint8_t), BUFF_SIZE);
    fifo_init(&mouse_fifo, mouse_buf, sizeof(uint8_t), BUFF_SIZE);
	fifo_init(&joystick_fifo, joystick_buf, sizeof(uint8_t), BUFF_SIZE);
}

void keyboard_write(uint8_t ch)
{
    uint32_t * write_addr = fifo_get_write_addr(&keyboard_fifo);
    *write_addr = ch;
    fifo_write_done(&keyboard_fifo);
}

uint8_t keyboard_read(void)
{
    uint32_t * read_addr = fifo_get_read_addr(&keyboard_fifo);
    fifo_read_done(&keyboard_fifo);
    return *read_addr;
}

void joystick_write(uint8_t ch)
{
    uint32_t * write_addr = fifo_get_write_addr(&joystick_fifo);
    *write_addr = ch;
    fifo_write_done(&joystick_fifo);
}

uint8_t joystick_read(void)
{
    uint32_t * read_addr = fifo_get_read_addr(&joystick_fifo);
    fifo_read_done(&joystick_fifo);
    return *read_addr;
}


int shift_x = 20; 
int shift_y = 20;

void console_render(void)
{
    uint8_t ch;
    char *str = "Hello RISC-V ";
    uint32_t w = de_layer_get_h();
    uint32_t h = de_layer_get_w();


    for (int i = 0; i < 12; i++)
    {
///drow char        
        uint8_t ch_size = 16;
        //uint8_t *bitmap = font8x8_basic[str[i]];
        uint8_t *bitmap = font16x16[str[i]];
        int x,y,b,set;
        for (y=0; y < ch_size; y++) {
            for (x=0; x < ch_size; x++) {
                    set = bitmap[(y * (ch_size / 8)) + (x / 8)] & (1 << 7 - (x - ((x / 8) * 8)));
                    if(set)
                        gr_draw_pixel(framebuffer, w, h, x + shift_x, y + shift_y, 0xffffffff);
                    else
                        gr_draw_pixel(framebuffer, w, h, x + shift_x, y + shift_y, 0xff00004f);
            }
        }
///   
        shift_x += ch_size;
        if(!(shift_x %= w)) shift_y += ch_size;
        shift_y %= h; 
    }

}