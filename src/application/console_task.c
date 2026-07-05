#include <platform.h>
#include <fifo.h>
#include <log.h>
#include <ccu.h>
#include <de.h>
#include <gr.h>
#include <g2d.h>
#include <font8x8_basic.h>
#include <font16x16.h>
#include <tlsf.h>
#include <tinyprintf.h>

#define BUFF_SIZE 100

extern tlsf_t mem_pool;

struct fifo_t keyboard_fifo;
struct fifo_t mouse_fifo;
struct fifo_t joystick_fifo;
struct fifo_t console_new_line_buf_fifo;
struct fifo_t console_string_buf_fifo;

uint8_t keyboard_buf[BUFF_SIZE];
uint8_t mouse_buf[BUFF_SIZE];
uint8_t joystick_buf[BUFF_SIZE];

uint8_t *console_string_buf;
uint32_t *console_new_line_buf;

extern uint8_t *framebuffer; 
extern struct g2d_rot_t g2d_rot_config;


uint16_t ch_size = 16;
uint8_t *font_buffer;

void console_render_font_buffer(void);

void console_new_line_buf_write(uint32_t elem);

void console_task_init(void)
{
    console_string_buf =  tlsf_malloc(mem_pool, 2000);
    console_new_line_buf =  tlsf_malloc(mem_pool, 100 * 4);

	fifo_init(&keyboard_fifo, keyboard_buf, sizeof(uint8_t), ARRAY_SIZE(keyboard_buf));
    fifo_init(&mouse_fifo, mouse_buf, sizeof(uint8_t), ARRAY_SIZE(mouse_buf));
	fifo_init(&joystick_fifo, joystick_buf, sizeof(uint8_t), ARRAY_SIZE(joystick_buf));

    fifo_init(&console_string_buf_fifo, console_string_buf, sizeof(uint8_t), 2000);
    fifo_init(&console_new_line_buf_fifo, console_new_line_buf, sizeof(uint32_t), 100);

    console_new_line_buf_write( fifo_get_read_addr(&console_string_buf_fifo));
    console_render_font_buffer();
}


void console_render_font_buffer(void)
{
    font_buffer = tlsf_memalign(mem_pool, 16, ch_size * ch_size * 4 * 128);

    for (int i = 0; i < 128; i++)
    {        
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
    //if(console_string_buf_fifo.write == console_string_buf_fifo.e_num - 1) fifo_reset(&console_string_buf_fifo);
}

uint8_t console_string_buf_read(void)
{
    uint32_t * read_addr = fifo_get_read_addr(&console_string_buf_fifo);
    fifo_read_done(&console_string_buf_fifo);
    return *read_addr;
}

void console_new_line_buf_write(uint32_t elem)
{
    uint32_t * write_addr = fifo_get_write_addr(&console_new_line_buf_fifo);
    *write_addr = elem;
    fifo_write_cyclic_done(&console_new_line_buf_fifo);
}

uint32_t console_new_line_buf_read(int offset)
{
    uint32_t * write_addr;

    if(console_new_line_buf_fifo.write >= offset)
        write_addr = ((uint32_t)console_new_line_buf_fifo.buffer + console_new_line_buf_fifo.write * console_new_line_buf_fifo.e_size) - (sizeof(uint32_t) * (offset));
    else 
        write_addr = ((uint32_t)console_new_line_buf_fifo.buffer + console_new_line_buf_fifo.e_num  * console_new_line_buf_fifo.e_size) - (sizeof(uint32_t) * (offset - console_new_line_buf_fifo.write));
    
    return *write_addr;
}

uint32_t w = 0;
uint32_t h = 0;
uint32_t w_margin = 10;
uint32_t h_margin = 10;
uint32_t shift_x = 0; 
uint32_t shift_y = 0;

void console_render_char(uint8_t symbol)
{
    int offset = symbol * ch_size * ch_size * 4;
    for (int y = 0; y < ch_size; y++) {
        for (int x = 0; x < ch_size; x++) {
            *(volatile uint32_t *)((uint32_t) framebuffer + 4 * ((y + shift_y + h_margin) * w + x + shift_x + w_margin)) = 
            *(volatile uint32_t *)((uint32_t) font_buffer + 4 * (y*ch_size + x) + offset);
        }
    }
}


void console_fill_string_init(void)
{
    for(int i; i < 2000;i++){

        char str_out[20];
        int num = tfp_sprintf(str_out, "String %d \n", i);

        for(int j = 0; j < num; j++)
        {
            console_string_buf_write(str_out[j]);
        }
        
    }
}

int str_a = 0;
int str_b = 0;
unsigned long ms2;

void console_fill_string(void)
{
    if (get_time_ms() > ms2 + 10)
    {
        ms2 = get_time_ms();

        if(str_a < 2000){

            char str_out[20];
            int num = tfp_sprintf(str_out, "String %d \n", str_a);

            if(str_b < num)
            {
                console_string_buf_write(str_out[str_b]);
                str_b++;
            }
            else 
            {
                str_b = 0;
                str_a++;
            }
        }
    }
}

int need_rerender = 0;

void console_render(void)
{
    w = de_layer_get_h();
    h = de_layer_get_w();
    uint32_t w_space = w - w_margin * 2;
    uint32_t h_space = h - w_margin * 2;
    uint32_t col_num = w_space / ch_size;
    uint32_t row_num = 10; //h_space / ch_size;

    while( fifo_get_available(&console_string_buf_fifo) && !need_rerender)
    {
        char symbol = console_string_buf_read();

        if(symbol > 31 && symbol < 127) { //normal symbol
            console_render_char(symbol);
            shift_x += ch_size;
        }

        if( (shift_x / ch_size) == col_num || symbol == '\n') { //new line by row end
            shift_x = 0; 
            shift_y += ch_size; 
            console_new_line_buf_write(console_string_buf_fifo.read);
        }
        
        if( (shift_y / ch_size) == row_num ) { //end screen
            console_string_buf_fifo.read = console_new_line_buf_read(row_num); // 2 row first symbol addres
            need_rerender = 1;
        }
    } 

    if(need_rerender){ //clean last row
        shift_y = ch_size * (row_num - 1); 
        for(int i; i < col_num; i++) 
        {
            console_render_char((char)' ');
            shift_x += ch_size;
        }
        shift_x = 0; 
        shift_y = 0; 
        need_rerender = 0;
    }
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

