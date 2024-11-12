#include <platform.h>
#include <tusb.h>
#include <log.h>
#include <irq.h>

#define EHCI0_BASE (0x04101000)
#define EHCI1_BASE (0x04200000)
#define OHCI_BASE  (0x04200000+0x400)

static void usb_hw_init(void);

void task_usb()
{
	LOG_D("usb_task");
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

static void usb_hw_init(void)
{
	volatile uint32_t *usb_ctrl = (uint32_t * ) (EHCI1_BASE + 0x800);
	volatile uint32_t *phy_ctrl = (uint32_t * ) (EHCI1_BASE + 0x810);
	volatile uint32_t *portsc   = (uint32_t * ) (EHCI1_BASE + 0x054);
	volatile uint32_t *confflag = (uint32_t * ) (EHCI1_BASE + 0x050);

	CCU->USB1_CLK_REG |= 0x01 << 24; // usb clock 12M divided from 24MHz 
	CCU->USB1_CLK_REG |= BV(30) | BV(31); // rst USB1 phy, gating

	CCU->USB_BGR_REG |= BV(21); // rst EHCI1
	CCU->USB_BGR_REG |= BV(5); // gating EHCI1
	CCU->USB_BGR_REG |= BV(17); // rst oHCI1
	CCU->USB_BGR_REG |= BV(1); // gating oHCI1

	*phy_ctrl &= ~BV(3);
	//*usb_ctrl &= ~BV(28);
	*usb_ctrl |= BV(10) | BV(9) | BV(8) | BV(0);

	//*confflag = 0;
	*portsc |= BV(13);

	//irq_assign(USB1_EHCI_IRQn, usb_int_handler);
	irq_assign(USB1_OHCI_IRQn, usb_int_handler);

}

/* bool hcd_init(uint8_t rhport)
{
  return ehci_init(rhport, (uint32_t) EHCI1_BASE, (uint32_t) EHCI1_BASE+0x10);
} */

void hcd_int_enable(uint8_t rhport)
{
	(void)rhport;
	//irq_enable(USB1_EHCI_IRQn);
	irq_enable(USB1_OHCI_IRQn);
}

void hcd_int_disable(uint8_t rhport)
{
	(void)rhport;
	//irq_disable(USB1_EHCI_IRQn);
	irq_disable(USB1_OHCI_IRQn);
}

