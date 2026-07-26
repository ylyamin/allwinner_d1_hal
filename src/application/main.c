#include <config.h>
#include <platform.h>
#include <log.h>
#include <ccu.h>
#include <led.h>
#include <usb_task.h>
#include <dispaly_task.h>
#include <console_task.h>
#include <command.h>
#include <doom_task.h>
#include <tlsf.h>

extern unsigned char __HeapBase; 
extern unsigned char __HeapLimit; 
tlsf_t mem_pool = NULL;

void main(void)
{
    size_t s = (size_t)(&__HeapLimit - &__HeapBase);
	LOG_W("heap: creating mem pool @ %08x size %d\n", &__HeapBase, s);
	mem_pool = tlsf_create_with_pool((void *)&__HeapBase, s);
    
    console_task_init();
    
    gui_init();
    
    display_task_init();	

    console_task_inputs_init();

    usb_task_init();

    int i;

    while(1)
    {
        if(i == 100)
        {
            run_command("doom");
        }
        i++;

        usb_task_exec(); // usb work only when dcache not enabled 
                
        console_render();    
        
        gui();
        
        console_command_handler();

        display_task_exec();	
    }
}



