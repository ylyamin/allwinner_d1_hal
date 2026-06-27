#ifndef CONSOLE_TASK_H_
#define CONSOLE_TASK_H_
#include <fifo.h>

void console_task_init(void);
void console_task_exec(void);

void keyboard_write(uint8_t ch);
uint8_t keyboard_read(void);

void joystick_write(uint8_t ch);
uint8_t joystick_read(void);

#endif /* CONSOLE_TASK_H_ */

