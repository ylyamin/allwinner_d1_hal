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
    
    //usb_task_init();

    display_task_init();	

    console_task_init(); //FIXME stack if before display

    console_task_inputs_init();

    //run_command("doom");
    
    while(1)
    {
        //usb_task_exec();
        console_fill_string();
        //console_command_handler();
        
        console_render();    
        
        display_task_exec();	
    }
}



