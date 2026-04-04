#ifndef CONSOLE_TASK_H_
#define CONSOLE_TASK_H_
#include <fifo.h>

void console_task_init(void);
void console_task_exec(void);
void console_write(uint8_t ch);
uint8_t console_read(void);

#endif /* CONSOLE_TASK_H_ */

