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
struct fifo_t console_new_line_buf_fifo;
struct fifo_t console_string_buf_fifo;

uint8_t keyboard_buf[BUFF_SIZE];
uint8_t mouse_buf[BUFF_SIZE];
uint8_t joystick_buf[BUFF_SIZE];
uint32_t console_new_line_buf[BUFF_SIZE];
uint8_t console_string_buf[2200];

extern uint8_t *framebuffer; 
uint16_t ch_size = 16;
uint8_t *font_buffer;

void console_render_font_buffer(void);

void console_task_init(void)
{
	fifo_init(&keyboard_fifo, keyboard_buf, sizeof(uint8_t), sizeof(keyboard_buf));
    fifo_init(&mouse_fifo, mouse_buf, sizeof(uint8_t), sizeof(mouse_buf));
	fifo_init(&joystick_fifo, joystick_buf, sizeof(uint8_t), sizeof(joystick_buf));
    fifo_init(&console_new_line_buf_fifo, console_new_line_buf, sizeof(uint32_t), sizeof(console_new_line_buf));
    fifo_init(&console_string_buf_fifo, console_string_buf, sizeof(uint8_t), sizeof(console_string_buf));

    console_render_font_buffer();
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

void console_new_line_buf_write(uint32_t elem)
{
    uint32_t * write_addr = fifo_get_write_addr(&console_new_line_buf_fifo);
    *write_addr = elem;
    fifo_write_cyclic_done(&console_new_line_buf_fifo);
}

uint32_t console_new_line_buf_addr_read(void)
{
    return fifo_get_write_addr(&console_new_line_buf_fifo) - sizeof(uint32_t);
}

uint32_t w = 0;
uint32_t h = 0;
uint32_t w_margin = 10;
uint32_t h_margin = 10;
uint32_t shift_x = 0; 
uint32_t shift_y = 0;
uint32_t need_rerender = 0;

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

uint64_t fstr = 0;
unsigned long ms1;

void console_fill_string(void)
{
    char *str = "Hello-RISC-V";

/*     if (get_time_ms() > ms1 + 20)
	{
		ms1 = get_time_ms();
 */
        for (int j = 0; j < 10; j++)//78*28
        {
            for (int i = 33; i < 110; i++)
            {
                console_string_buf_write(i);
            }
        }

            for (int i = 33; i < 44; i++)
            {
                console_string_buf_write(i);
            }

/*     } */
}

void console_render(void)
{
    w = de_layer_get_h();
    h = de_layer_get_w();
    uint32_t w_space = w - w_margin * 2;
    uint32_t h_space = h - w_margin * 2;
    uint32_t col_num = w_space / ch_size;
    uint32_t row_num = h_space / ch_size;
    uint8_t symbol;

    if(fifo_empty(&console_string_buf_fifo) != 1)
    {
        if(console_string_buf_fifo.read == 0) console_new_line_buf_write( fifo_get_read_addr(&console_string_buf_fifo)); //add 1 row first symbol addres
/* 
        while(fifo_get_read_size_cont(&console_string_buf_fifo))
        { */
            if(need_rerender) { //shift screen up


                while((uint32_t)need_rerender != (uint32_t)fifo_get_read_addr(&console_string_buf_fifo))
                {
                    symbol = (char) *((uint32_t *)need_rerender);
                    console_render_char(symbol);
                    need_rerender = need_rerender + sizeof(uint8_t);

                    shift_x += ch_size;
                    if((shift_x / ch_size) == col_num) { //new line by row end
                        shift_x = 0; 
                        shift_y += ch_size; 
                    }
                }

                for(int i = 0; i++; i < col_num)
                {
                    console_render_char("_");
                    shift_x += ch_size;
                }

                need_rerender = 0;
            }
            else
            {

                symbol = console_string_buf_read();
                console_render_char(symbol);


                shift_x += ch_size;
                if((shift_x / ch_size) == col_num) { //new line by row end
                    shift_x = 0; 
                    shift_y += ch_size; 
                    console_new_line_buf_write( fifo_get_read_addr(&console_string_buf_fifo));
                }
                
                if((shift_y / ch_size) == 10) { ////end screen
                    shift_y = 0; 
                    uint32_t * addr = console_new_line_buf_addr_read() - (sizeof(uint32_t) * 9); // 2 row first symbol addres
                    need_rerender = *addr; 
                }
            }
    }
}