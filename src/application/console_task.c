#include <platform.h>
#include <console_task.h>
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

#define BG_COLOR_1 0xff00004f
#define FONT_COLOR_1 0xffffffff
#define STRING_BUFF_SIZE 2000
#define NEW_LINE_BUFF_SIZE 300

extern tlsf_t mem_pool;
extern uint8_t *framebuffer; 

struct fifo_t console_new_line_buf_fifo;
struct fifo_t console_string_buf_fifo;
uint8_t *console_string_buf;
uint32_t *console_new_line_buf;
uint8_t *font_buffer;

uint16_t ch_size = 16;
uint32_t w = 0;
uint32_t h = 0;
uint32_t w_margin = 10;
uint32_t h_margin = 10;
uint32_t shift_x = 0; 
uint32_t shift_y = 0;
uint32_t col_num;
uint32_t row_num;
uint8_t console_task_init_done = 0;

void console_task_init(void)
{
    console_string_buf =  tlsf_malloc(mem_pool, STRING_BUFF_SIZE);
    console_new_line_buf =  tlsf_malloc(mem_pool, NEW_LINE_BUFF_SIZE * 4);

    fifo_init(&console_string_buf_fifo, console_string_buf, sizeof(uint8_t), STRING_BUFF_SIZE);
    fifo_init(&console_new_line_buf_fifo, console_new_line_buf, sizeof(uint32_t), NEW_LINE_BUFF_SIZE);
    console_new_line_buf_write( fifo_get_read_addr(&console_string_buf_fifo));
    console_render_font_buffer();
    console_task_init_done = 1;
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
                        *(volatile uint32_t *)((uint32_t) font_buffer + 4 * (y*ch_size + x) + offset) = FONT_COLOR_1;
                    else
                        *(volatile uint32_t *)((uint32_t) font_buffer + 4 * (y*ch_size + x) + offset) = BG_COLOR_1;
            }
        }        
    }
}

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

uint32_t max_col_num;

void console_clean_row(uint32_t shift_x)
{
    max_col_num = MAX(max_col_num, shift_x);

    for (int y = 0; y < ch_size; y++) {
        for (int x = 0; x < max_col_num - shift_x; x++) {
            *(volatile uint32_t *)((uint32_t) framebuffer + 4 * ((y + shift_y + h_margin) * w + x + shift_x + w_margin)) = BG_COLOR_1;
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

void console_render(void)
{
    w = de_layer_get_h();
    h = de_layer_get_w();
    col_num = (w - w_margin * 2) / ch_size;
    row_num = (h - h_margin * 2) / ch_size;

    while(fifo_get_available(&console_string_buf_fifo))
    {
        char symbol = console_string_buf_read();

        if(symbol > 31 && symbol < 127) { //normal symbol
            console_render_char(symbol);
            shift_x += ch_size;
        }
        
        if(symbol == '\033') while(console_string_buf_read() != 'm'); //skip color modifier
 
        if(symbol == '\r') shift_x = 0; //carriage return

        if( ((shift_x / ch_size) == col_num) || symbol == '\n') { //new line by row end
            console_clean_row(shift_x); //clean after \n
            shift_x = 0; 
            shift_y += ch_size; 
            console_new_line_buf_write(console_string_buf_fifo.read);
        }
        
        if( (shift_y / ch_size) == row_num ) { //end screen
            console_string_buf_fifo.read = console_new_line_buf_read(row_num); // 2 row first symbol addres            
            shift_y -= ch_size; 
            console_clean_row(shift_x); //clean last row
            max_col_num = 0;
            shift_y = 0; 
            return;
        }
    } 
}

void console_fill_string_init(void)
{
    for(int i; i < 200;i++){

        char str_out[20];
        int num = tfp_sprintf(str_out, "String %d \n\n", i);

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

        if(str_a < 5){

            char str_out[20];
            int num = tfp_sprintf(str_out, "String %d \n\n", str_a);

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

#define BUFF_SIZE 100

struct fifo_t keyboard_fifo;
struct fifo_t mouse_fifo;
struct fifo_t joystick_fifo;
uint8_t keyboard_buf[BUFF_SIZE];
uint8_t mouse_buf[BUFF_SIZE];
uint8_t joystick_buf[BUFF_SIZE];

void console_task_inputs_init(void)
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

uint8_t command_string[100];
int command_string_num;

void console_command_handler(void)
{
    while(fifo_get_available(&keyboard_fifo))
    {
        uint8_t symbol = keyboard_read();

        tfp_printf("%c",symbol);
        console_string_buf_write(symbol);

        if (symbol == '\r' || command_string_num > 99) //Enter 
        {
            command_string_num = 0;
            LOG_E("\n Command: %s not found", command_string);
        } else
        {
            command_string[command_string_num] = symbol;
            command_string_num++;
        }

    }
}