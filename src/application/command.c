#include <platform.h>
#include <command.h>
#include <console_task.h>
#include <log.h>
#include <tinyprintf.h>
#include <string.h>

void cmd_help(void);
extern void logo(void);

commands_t commands[] = {
    {.cmd = "help",     .hlp = "show commands", .func = cmd_help,   },
    {.cmd = "version",  .hlp = "show version",  .func = logo,       },
};

void cmd_help(void)
{
    small_printf("List of commands:\n");
    for(int i = 0; i < ARRAY_SIZE(commands); i++)
        small_printf("%5s - %s\n",commands[i].cmd,commands[i].hlp);
}

void console_find_command(const char *str, size_t n)
{
    int cmd_found, cmd_num;
    if(str[0])
    {
        for(int i = 0; i < ARRAY_SIZE(commands); i++)
        {
            if(strncmp(commands[i].cmd, str, n) == 0)
            { cmd_found = 1; cmd_num = i; }
        }

        if(cmd_found)
        {
            small_printf("\n");
            commands[cmd_num].func();
        }
        else
            small_printf("\nCommand: '%s' not found. Use 'help'", str);
    }
}

uint8_t command_string[BUFF_SIZE];
int command_string_num;
int command_welcome = 1;

void console_command_handler(void)
{
    if(command_welcome)
    {
        small_printf(WELCOME);
        command_welcome = 0;
    }

    while(fifo_get_available(&keyboard_fifo))
    {
        uint8_t symbol = keyboard_read();
        
        if (symbol == '\x1b') symbol = '\0'; //Esc ignore
        
        if (symbol == '\r') //Enter 
        {   
            console_find_command(command_string, command_string_num);
            small_printf("\n");
            small_printf(WELCOME);

            //flush command
            for(int i = 0; i < command_string_num; i++) command_string[i] = 0;
            command_string_num = 0;
        } 
        else
        {
            if(command_string_num < BUFF_SIZE)
            {
                command_string[command_string_num] = symbol;
                command_string_num++;
            }
            small_printf("%c",symbol);
        }
    }
}