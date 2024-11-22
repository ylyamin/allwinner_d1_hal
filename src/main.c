#include <config.h>
#include <platform.h>
#include <log.h>
#include <ccu.h>
#include <led.h>
#include <usb_task.h>
#include <dispaly_task.h>

void main(void)
{

#ifdef CONFIG_USE_USB
	usb_task_init();
#endif

#ifdef CONFIG_USE_DISPLAY
	display_task_init();	
#endif

    while(1)
    {

    #ifdef CONFIG_USE_USB
        usb_task_exec();
    #endif

    #ifdef CONFIG_USE_DISPLAY
        display_task_exec();	
    #endif
    
    }
}