#ifndef COMMAND_H_
#define COMMAND_H_

#define BUFF_SIZE 100
#define WELCOME ">"

typedef struct
{
    char *cmd;
    char *hlp;
    void (*func)(void);    
} commands_t;

extern struct fifo_t keyboard_fifo;

void run_command(const char *str);
void console_command_handler(void);

#endif /* COMMAND_H_ */