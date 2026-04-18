#include <platform.h>
#include <fifo.h>
#include <log.h>
#include <ccu.h>
#include <de.h>
#include <gr.h>
#include <font8x8_basic.h>

#define CONSOLE_BUF 4096
struct fifo_t console_fifo;
uint8_t console_buf[CONSOLE_BUF];

extern uint8_t framebuffer[]; 

void console_task_init(void)
{
	fifo_init(&console_fifo, console_buf, sizeof(uint8_t), CONSOLE_BUF);
}

void console_write(uint8_t ch)
{
    uint32_t * write_addr = fifo_get_write_addr(&console_fifo);
    *write_addr = ch;
    fifo_write_done(&console_fifo);
}

uint8_t console_read(void)
{
    uint32_t * read_addr = fifo_get_read_addr(&console_fifo);
    fifo_read_done(&console_fifo);
    return *read_addr;
}

int shift_x, shift_y;

void console_task_exec(void)
{
    uint8_t ch;
    uint32_t h = de_layer_get_h();
    uint32_t w = de_layer_get_w();

    if(fifo_empty(&console_fifo) != 1)
    {
        ch = console_read();
        LOG_I("%c",ch);
    
        char *bitmap = font8x8_basic[ch];
        int x,y,set;
        for (x=0; x < 8; x++) {
            for (y=0; y < 8; y++) {
                set = bitmap[x] & 1 << y;
                if(set)
                    gr_draw_pixel(&framebuffer, w, h, y + shift_x, x + shift_y, 0xffffffff);
                else
                    gr_draw_pixel(&framebuffer, w, h, y + shift_x, x + shift_y, 0xff0000ff);
            }
        }
        shift_x += 10;
        if(!(shift_x %= w)) shift_y +=10;
        shift_y %= h;
    }
}