#include <config.h>
#include <platform.h>
#include <log.h>
#include <ccu.h>
#include <led.h>
#include <usb_task.h>
#include <dispaly_task.h>
#include <console_task.h>
#include <doomgeneric.h>

void main(void)
{
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
        char argv[0]; 
        doomgeneric_Create(0, *argv);
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
            console_task_exec();
        #endif

        #ifdef CONFIG_USE_DOOM
            doomgeneric_Tick(); 
        #endif

    }
}