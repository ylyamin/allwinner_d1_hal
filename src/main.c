#include <config.h>
#include <platform.h>
#include <log.h>
#include <ccu.h>
#include <led.h>
#include <usb_task.h>
#include <dispaly_task.h>
#include <console_task.h>
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

    #ifdef CONFIG_USE_USB
        usb_task_init();
    #endif

    #ifdef CONFIG_USE_DISPLAY
        display_task_init();	
    #endif

    #ifdef CONFIG_USE_CONSOLE
        console_task_init();
    #endif

    #ifdef CONFIG_USE_DOOM
        doom_task_init();
    #endif

    while(1)
    {

        #ifdef CONFIG_USE_USB
            usb_task_exec();
        #endif

        #ifdef CONFIG_USE_DISPLAY
            display_task_exec();	
        #endif

        #ifdef CONFIG_USE_CONSOLE
            //console_task_exec();
        #endif

        #ifdef CONFIG_USE_DOOM
            doom_task_exec();
        #endif

    }
}