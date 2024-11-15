#ifndef AXP_H_
#define AXP_H_

#include "platform.h"

   void axp_set_reg(TWI_TypeDef *twi, uint8_t reg_addr, uint8_t reg_value);
uint8_t axp_get_reg(TWI_TypeDef *twi, uint8_t reg_addr);

void axp_get_enable(TWI_TypeDef *twi);
void axp_get_voltage(TWI_TypeDef *twi);

void axp_USB_control(TWI_TypeDef *twi, int on_off);
void axp_LCD_control(TWI_TypeDef *twi, int on_off);

#endif /* AXP_H_ */