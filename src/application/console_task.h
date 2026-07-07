#ifndef CONSOLE_TASK_H_
#define CONSOLE_TASK_H_
#include <fifo.h>

void console_task_init(void);
void console_task_inputs_init(void);

void console_render(void);
void console_render_font_buffer(void);
void console_new_line_buf_write(uint32_t elem);
void console_string_buf_write(uint8_t ch);

void console_fill_string(void);
void console_fill_string_init(void);

void keyboard_write(uint8_t ch);
uint8_t keyboard_read(void);

void joystick_write(uint8_t ch);
uint8_t joystick_read(void);

#endif /* CONSOLE_TASK_H_ */

