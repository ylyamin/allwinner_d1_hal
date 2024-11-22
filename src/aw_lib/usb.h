#ifndef USB_H_
#define USB_H_
#include <platform.h>
#include <tusb.h>

void usb_hw_init(void);
bool hcd_init(uint8_t rhport);
void usb_int_handler(void);
void hcd_int_enable(uint8_t rhport);
void hcd_int_disable(uint8_t rhport);

#endif /* USB_H_ */