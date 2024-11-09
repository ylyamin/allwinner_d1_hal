#include "platform.h"

//#include "irq.h"
//#include "FreeRTOS.h"
//#include "task.h"

#include "tusb.h"
#include "portable/ehci/ehci_api.h"

//TaskHandle_t task_usb_handle;

#define EHCI0_BASE (0x04101000)
#define EHCI1_BASE (0x04200000)
#define OHCI_BASE (0x04200000+0x400)

static void usb_hw_init(void);

void task_usb()
{
	small_printf("task: usb\n");

	usb_hw_init();

	tuh_init(0);

	while(1)
	{
		tuh_task();
	}
}

void usb_int_handler(void)
{
	tuh_int_handler(0);
}

/* void usb_task_init(void)
{

	BaseType_t ret = xTaskCreate(task_usb, "usb", 1000, NULL, tskIDLE_PRIORITY+2, &task_usb_handle);
	if (ret != pdTRUE){
		uart_printf("not created\n");
		while(1);
	}
} */

static void usb_hw_init(void)
{
	volatile uint32_t *usb_ctrl = (uint32_t * ) (EHCI1_BASE + 0x800);
	volatile uint32_t *phy_ctrl = (uint32_t * ) (EHCI1_BASE + 0x810);
	volatile uint32_t *portsc  = (uint32_t * ) (EHCI1_BASE + 0x054);


	CCU->USB1_CLK_REG |= 0x01 << 24; // usb clock from 24MHz
	CCU->USB1_CLK_REG |= BV(30) | BV(31); // rst USB1 phy, gating

	CCU->USB_BGR_REG |= BV(21); // rst EHCI1
	CCU->USB_BGR_REG |= BV(5); // gating EHCI1
	CCU->USB_BGR_REG |= BV(17); // rst oHCI1
	CCU->USB_BGR_REG |= BV(1); // gating oHCI1

	*phy_ctrl &= ~BV(3);
	*usb_ctrl |= BV(10) | BV(9) | BV(8) | BV(0);

	small_printf("phy_ctl = %x\n", *phy_ctrl);
	small_printf("usb_ctl = %x\n", *usb_ctrl);
	small_printf("portsc = %x\n", *portsc);

	*portsc |= BV(13);


	//irq_set_handler(USB1_OHCI_IRQn, usb_int_handler, NULL);
	//irq_set_prio(USB1_OHCI_IRQn, configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );

}

void hcd_int_enable(uint8_t rhport)
{
	(void)rhport;
	irq_enable(USB1_OHCI_IRQn);
}

/*
bool hcd_init(uint8_t rhport)
{
  return ehci_init(rhport, (uint32_t) EHCI1_BASE, (uint32_t) EHCI1_BASE+0x10);
}

void hcd_int_enable(uint8_t rhport)
{
	(void)rhport;
	irq_set_enable(USB1_EHCI_IRQn, 1);
}
*/

void hcd_int_disable(uint8_t rhport)
{
	(void)rhport;
	irq_disable(USB1_OHCI_IRQn);
}

