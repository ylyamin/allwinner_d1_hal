#include <platform.h>
#include <usb.h>
#include <tusb.h>
#include <tusb_config.h>
#include "portable/ehci/ehci_api.h"
#include <log.h>
#include <irq.h>

#define EHCI0_BASE (0x04101000)
#define EHCI1_BASE (0x04200000)
#define OHCI_BASE  (0x04200000+0x400)

void usb_hw_init(void)
{
	volatile uint32_t *usb_ctrl = (uint32_t * ) (EHCI1_BASE + 0x800);
	volatile uint32_t *phy_ctrl = (uint32_t * ) (EHCI1_BASE + 0x810);
	volatile uint32_t *portsc   = (uint32_t * ) (EHCI1_BASE + 0x054);
	volatile uint32_t *confflag = (uint32_t * ) (EHCI1_BASE + 0x050);

	CCU->USB1_CLK_REG &= ~(BV(24) | BV(25));
	CCU->USB1_CLK_REG |= BV(24); // usb clock 12M divided from 24MHz 
	CCU->USB1_CLK_REG |= BV(30) | BV(31); // rst USB1 phy, gating

	CCU->USB_BGR_REG &= ~BV(17);	// rst OHCI1
	CCU->USB_BGR_REG &= ~BV(21);	// rst EHCI1

	CCU->USB_BGR_REG |= BV(21); // rst EHCI1
	CCU->USB_BGR_REG |= BV(5); // gating EHCI1

	CCU->USB_BGR_REG |= BV(17); // rst OHCI1
	CCU->USB_BGR_REG |= BV(1); // gating OHCI1


	*phy_ctrl &= ~BV(3);
	*usb_ctrl |= BV(11) | BV(10) | BV(9) | BV(8) | BV(0);

#ifdef TUP_USBIP_EHCI

	*confflag = 1;
	*portsc &= ~BV(13);
	irq_assign(USB1_EHCI_IRQn, (void *) usb_int_handler);

#elif TUP_USBIP_OHCI

	*confflag = 0;
	*portsc |= BV(13);
	irq_assign(USB1_OHCI_IRQn, (void *) usb_int_handler);

#endif
}

void usb_int_handler(void)
{
	tuh_int_handler(0);
}

#ifdef TUP_USBIP_EHCI
bool hcd_init(uint8_t rhport)
{
  return ehci_init(rhport, (uint32_t) EHCI1_BASE, (uint32_t) EHCI1_BASE+0x10);
}
#endif

void hcd_int_enable(uint8_t rhport)
{
	(void)rhport;

#ifdef TUP_USBIP_EHCI
	irq_enable(USB1_EHCI_IRQn);
#elif TUP_USBIP_OHCI
	irq_enable(USB1_OHCI_IRQn);
#endif
}

void hcd_int_disable(uint8_t rhport)
{
	(void)rhport;
#ifdef TUP_USBIP_EHCI
	irq_disable(USB1_EHCI_IRQn);
#elif TUP_USBIP_OHCI
	irq_disable(USB1_OHCI_IRQn);
#endif
}
