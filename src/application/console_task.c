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
#include <tcon_lcd.h>
#include <thermal.h>
#include <axp228.h>

#define BG_COLOR_1 0x0f00004f
#define BG_COLOR_2 0x0f00004f0f00004f
#define FONT_COLOR_1 0x0fffffff
#define STRING_BUFF_SIZE 3000
#define NEW_LINE_BUFF_SIZE 300
#define FB_MULTIPLY 2

extern tlsf_t mem_pool;
extern uint8_t *framebuffer; 
extern struct g2d_rot_t g2d_rot_config;
extern timing_t LCD_get_param(void);

struct fifo_t console_new_line_buf_fifo;
struct fifo_t console_string_buf_fifo;
uint8_t *console_string_buf;
uint32_t *console_new_line_buf;
uint8_t *font_buffer;
uint8_t *console_framebuffer; 
uint8_t *gui_framebuffer; 

uint16_t ch_size = 16;
uint32_t w = 0;
uint32_t h = 0;
uint32_t w_margin = 16;
uint32_t h_margin = 32;
uint32_t shift_x = 0; 
uint32_t shift_y = 0;
uint32_t col_num = 0;
uint32_t row_num = 0;
uint32_t row_num_max = 0;
uint8_t console_task_init_done = 0;

void console_task_init(void)
{
    timing_t timing = LCD_get_param();
    w = timing.lcd_scale_h;
    h = timing.lcd_scale_w;

    col_num = (w - w_margin * 2) / ch_size; // margin left and right
    row_num = (h - h_margin) / ch_size; // margin only top
    row_num_max = row_num * FB_MULTIPLY;

    console_string_buf =  tlsf_malloc(mem_pool, STRING_BUFF_SIZE);
    console_new_line_buf =  tlsf_malloc(mem_pool, NEW_LINE_BUFF_SIZE);

    console_framebuffer =  tlsf_memalign(mem_pool, 16, w * h * 4 * FB_MULTIPLY);
    gr_fill(console_framebuffer, w, h * FB_MULTIPLY, BG_COLOR_1);

    fifo_init(&console_string_buf_fifo, console_string_buf, sizeof(uint8_t), STRING_BUFF_SIZE);
    fifo_init(&console_new_line_buf_fifo, console_new_line_buf, sizeof(uint32_t), NEW_LINE_BUFF_SIZE);

    console_new_line_buf_write((uint32_t) console_framebuffer );
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

void console_render_char(uint8_t symbol, int32_t shift_x, int32_t shift_y, void *fb)
{
    int offset = symbol * ch_size * ch_size * 4;
    int offset_y = shift_y + h_margin;
    int offset_x = shift_x + w_margin;

    for (int y = 0; y < ch_size; y++) {
        for (int x = 0; x < ch_size; x += 2) {
            *(volatile uint64_t *)((uint32_t) fb + 4 * ((y + offset_y) * w + x + offset_x)) = 
            *(volatile uint64_t *)((uint32_t) font_buffer + 4 * (y*ch_size + x) + offset);
        }
    }
}

uint32_t console_fb_addr(void){
    return (uint32_t)(console_framebuffer + 4 * (shift_y * w + shift_x));
}

uint32_t max_col_num;

void console_clean_row(uint32_t shift_x, uint32_t shift_y)
{
    int offset_y = shift_y + h_margin;
    int offset_x = shift_x + w_margin;
    max_col_num = MAX(max_col_num, shift_x); //remember max row width

    for (int x = 0; x < max_col_num - shift_x; x += 2) {
        for (int y = 0; y < ch_size; y++) {
            *(volatile uint64_t *)((uint32_t) console_framebuffer + 4 * ((y + offset_y) * w + x + offset_x)) = BG_COLOR_2;
        }
    }
}

void console_clean_end_row(void)
{
    int offset_y = (shift_y + h_margin) * w;

    for (int y = offset_y ; y < offset_y + (ch_size * w); y += 2) {
        *(volatile uint64_t *)((uint32_t) console_framebuffer + 4 * y) = BG_COLOR_2;
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
    while(fifo_get_available(&console_string_buf_fifo))
    {
        char symbol = console_string_buf_read();

        if(symbol > 31 && symbol < 127) { //normal symbol
            console_render_char(symbol, shift_x, shift_y, console_framebuffer);

            //if buffer close to end then also copy to start of buffer
            if( (shift_y / ch_size) >= row_num_max - row_num + 1 ) {
                console_render_char(symbol, shift_x, shift_y - (ch_size * (row_num_max - row_num + 1)), console_framebuffer);
            }

            shift_x += ch_size;
        }
        
        if((symbol == '\b') && (shift_x > ch_size)) { //backspace
            shift_x -= ch_size; 
            console_render_char(' ', shift_x, shift_y, console_framebuffer);
        }

        if(symbol == '\033') while(console_string_buf_read() != 'm'); //skip color modifier
 
        if(symbol == '\r') shift_x = 0; //carriage return

        if( ((shift_x / ch_size) == col_num) || symbol == '\n') { //new line by row end
            
                console_clean_row(shift_x, shift_y); //clean after \n

            //if buffer close to end then also copy clean to start of buffer
            if( (shift_y / ch_size) >= row_num_max - row_num + 1 ) {
                console_clean_row(shift_x, shift_y - (ch_size * (row_num_max - row_num + 1)));
            }

            shift_x = 0; 
            shift_y += ch_size; 
            console_new_line_buf_write(console_fb_addr()); //save addr next row
        }

        if( (shift_y / ch_size) >= row_num ) { //end screen
            g2d_rot_config.src_fb = console_new_line_buf_read(row_num); // jump to second row addres  
            if(shift_x == 0) console_clean_row(shift_x, shift_y); //clean previous row
        }

        if( (shift_y / ch_size) == row_num_max ) { //end buffer back to first screen
            shift_y = (row_num - 1) * ch_size;   
            console_new_line_buf_fifo.write = (row_num);
            if(shift_x == 0) console_clean_row(shift_x, shift_y);
            g2d_rot_config.src_fb = console_framebuffer; 
            return;
        }
    } 
}

void gui_init(void)
{
    timing_t timing = LCD_get_param();
    w = timing.lcd_scale_h;
    h = timing.lcd_scale_w;
    
    gui_framebuffer =  tlsf_memalign(mem_pool, 16, w * 20 * 4);
    gr_fill(gui_framebuffer, w, 20, BG_COLOR_1);
	gr_draw_hline_xyw(gui_framebuffer, w, h, /*x*/ 10, /*y*/ 18, /*ww*/ w - 20, 0xffffffff);

    ths_init();
}

void gui(void)
{
    uint32_t gui_shift_x;
    int num1,num2;
    char str_out1[10], str_out2[20];

    num1 = tfp_sprintf(str_out1, "Terminal:");
    num2 = tfp_sprintf(str_out2, "Tem:%2dC Bat:%2d%%", ths_get_temp(), axp_battery_get_soc(TWI0));

    for(int j = 0; j < num1; j++)
    {
        console_render_char(str_out1[j], gui_shift_x, -h_margin, gui_framebuffer);
        gui_shift_x += ch_size;
    }

    for(int j = 0; j < col_num - num1 - num2; j++)
    {
        console_render_char(' ', gui_shift_x, -h_margin, gui_framebuffer);
        gui_shift_x += ch_size;
    }

    for(int j = 0; j < num2; j++)
    {
        console_render_char(str_out2[j], gui_shift_x, -h_margin, gui_framebuffer);
        gui_shift_x += ch_size;
    }
}

//load test

void console_fill_string_init(void)
{
    for(int i = 1; i < 300;i++){

        char str_out[70];
        int num = tfp_sprintf(str_out, "String %d \n", i);

        for(int j = 0; j < num; j++)
        {
            console_string_buf_write(str_out[j]);
        }
        
    }
}

int str_a = 1;
int str_b = 0;
unsigned long ms2;

void console_fill_string(void)
{
    if (get_time_ms() > ms2 + 5)
    {
        ms2 = get_time_ms();

        if(str_a < 1000){

            char str_out[70];
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

void perf(const char * str, void (*func)())
{
    unsigned long us;
    us = get_time_us();
    func();
    tfp_printf("%s: %d\n", str, get_time_us() - us);
}
