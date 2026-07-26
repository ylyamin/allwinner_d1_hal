#include <platform.h>
#include <config.h>
#include <usb.h>
#include <tusb.h>
#include <tusb_config.h>
#include <log.h>
#include <axp228.h>

void usb_task_init(void)
{
	LOG_D("usb_task_init");
#if (PLATFORM == PLATFORM_DEVTERM)
	axp_USB_control(TWI0,1);
#endif
	usb_hw_init();
	tuh_init(0);
}

void usb_task_exec(void)
{
	tuh_task();
}
