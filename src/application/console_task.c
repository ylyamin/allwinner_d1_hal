#include <platform.h>
#include <fifo.h>
#include <log.h>
#include <ccu.h>
#include <de.h>
#include <gr.h>
#include <font8x8_basic.h>

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


int shift_x, shift_y;

void console_render(void)
{
    uint8_t ch;
    char *str = "Hello RISC-V";
    uint32_t w = de_layer_get_h();
    uint32_t h = de_layer_get_w();

/*     if(fifo_empty(&keyboard_fifo) != 1)
    {
        ch = keyboard_read();
        LOG_I("%c",ch); */
    for (int i = 0; i < 12; i++)
    {
        char *bitmap = font8x8_basic[str[i]];
        int x,y,set;
        for (x=0; x < 8; x++) {
            for (y=0; y < 8; y++) {
                set = bitmap[x] & 1 << y;
                if(set)
                    gr_draw_pixel(framebuffer, w, h, y + shift_x, x + shift_y, 0xffffffff);
                else
                    gr_draw_pixel(framebuffer, w, h, y + shift_x, x + shift_y, 0xff0000ff);
            }
        }
        shift_x += 10;
        if(!(shift_x %= w)) shift_y +=10;
        shift_y %= h; 
    }
/*     } */
}