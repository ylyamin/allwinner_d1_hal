#include <platform.h>
#include <fifo.h>
#include <log.h>
#include <ccu.h>
#include <de.h>
#include <gr.h>
#include <font8x8_basic.h>
#include <font16x16.h>
#include <tlsf.h>

#define BUFF_SIZE 100

extern tlsf_t mem_pool;

struct fifo_t keyboard_fifo;
struct fifo_t mouse_fifo;
struct fifo_t joystick_fifo;
struct fifo_t console_row_buf_fifo;
struct fifo_t console_string_buf_fifo;

uint8_t keyboard_buf[BUFF_SIZE];
uint8_t mouse_buf[BUFF_SIZE];
uint8_t joystick_buf[BUFF_SIZE];
uint16_t console_row_buf[BUFF_SIZE][2];
uint8_t console_string_buf[2200];

extern uint8_t *framebuffer; 

void console_task_init(void)
{
	fifo_init(&keyboard_fifo, keyboard_buf, sizeof(uint8_t), sizeof(keyboard_buf));
    fifo_init(&mouse_fifo, mouse_buf, sizeof(uint8_t), sizeof(mouse_buf));
	fifo_init(&joystick_fifo, joystick_buf, sizeof(uint8_t), sizeof(joystick_buf));
    fifo_init(&console_row_buf_fifo, console_row_buf, sizeof(uint16_t), sizeof(console_row_buf));
    fifo_init(&console_string_buf_fifo, console_string_buf, sizeof(uint8_t), sizeof(console_string_buf));
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

uint8_t ch_size = 16;
uint8_t *font_buffer;

void console_render_font_buffer(void)
{
    font_buffer = tlsf_memalign(mem_pool, 16, ch_size * ch_size * 4 * 128);

    for (int i = 0; i < 128; i++)
    {        
        //bitmap = font8x8_basic[i];
        int offset = i * ch_size * ch_size * 4;
        for (int y = 0; y < ch_size; y++) {
            for (int x = 0; x < ch_size; x++) {
                    if(font16x16[i][(y * (ch_size / 8)) + (x / 8)] & (1 << 7 - (x - ((x / 8) * 8))))
                        *(volatile uint32_t *)((uint32_t) font_buffer + 4 * (y*ch_size + x) + offset) = 0xffffffff;
                    else
                        *(volatile uint32_t *)((uint32_t) font_buffer + 4 * (y*ch_size + x) + offset) = 0xff00004f;
            }
        }        
    }
}

void console_string_buf_write(uint8_t ch)
{
    uint32_t * write_addr = fifo_get_write_addr(&console_string_buf_fifo);
    *write_addr = ch;
    fifo_write_done(&console_string_buf_fifo);
}

uint8_t console_string_buf_read(void)
{
    uint32_t * read_addr = fifo_get_read_addr(&console_string_buf_fifo);
    fifo_read_done(&console_string_buf_fifo);
    return *read_addr;
}

void console_row_buf_write(uint16_t * elem)
{
    uint32_t * write_addr = fifo_get_write_addr(&console_row_buf_fifo);
    *write_addr = elem;
    fifo_write_done(&console_row_buf_fifo);
}

void *console_row_buf_read(void)
{
    uint32_t * read_addr = fifo_get_read_addr(&console_row_buf_fifo);
    fifo_read_done(&console_row_buf_fifo);
    return *read_addr;
}



void console_render(void)
{
    uint32_t w = de_layer_get_h();
    uint32_t h = de_layer_get_w();

    uint32_t w_margin = 10;
    uint32_t h_margin = 10;
    uint32_t w_space = w - w_margin * 2;
    uint32_t h_space = h - w_margin * 2;
    uint32_t col_num = w_space / ch_size;
    uint32_t row_num = h_space / ch_size;
    static uint32_t shift_x = 0; 
    static uint32_t shift_y = 0;

    console_render_font_buffer();

    char *str = " Hello RISC-V ";
///
    for (int j = 0; j < ((col_num * row_num) / 13); j++)
    {
        for (int i = 0; i < 13; i++)
        {
            console_string_buf_write(str[i]);
        }
    }
///

    uint16_t row_header[2];
    uint16_t *out_elem;

    for (uint16_t i = 0; i < 5; i++) {

        row_header[0] = i;
        row_header[1] = i*10;

        console_row_buf_write(row_header);

        out_elem = console_row_buf_read();

        LOG_E("elem: %d, %d \r\n",out_elem[0],out_elem[1]);
    }

    if(fifo_empty(&console_string_buf_fifo) != 1)
    {
        //first symbol
        //uint32_t * read_addr = fifo_get_read_addr(&console_string_buf_fifo);
        //row_header[0] = 0;

        while(fifo_get_read_size_cont(&console_string_buf_fifo))
        {
        ///render char
                int offset = console_string_buf_read() * ch_size * ch_size * 4;
                for (int y = 0; y < ch_size; y++) {
                    for (int x = 0; x < ch_size; x++) {
                        *(volatile uint32_t *)((uint32_t) framebuffer + 4 * ((y + shift_y + h_margin) * w + x + shift_x + w_margin)) = 
                        *(volatile uint32_t *)((uint32_t) font_buffer + 4 * (y*ch_size + x) + offset);
                    }
                }
        ///
                shift_x += ch_size;
                if((shift_x / ch_size) == col_num) { shift_x = 0; shift_y += ch_size; };
                if((shift_y / ch_size) == row_num) { shift_y = 0; };
        }
    }
}