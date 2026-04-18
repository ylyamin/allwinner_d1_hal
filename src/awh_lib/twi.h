#ifndef TWI_H_
#define TWI_H_
#include "platform.h"

void twi_init(TWI_TypeDef *twi, uint32_t freq);
void i2c_master_set(TWI_TypeDef *twi, uint8_t dev_addr, uint8_t reg_addr, uint8_t value);
uint8_t i2c_master_get(TWI_TypeDef *twi, uint8_t dev_addr, uint8_t reg_addr);

#endif /* TWI_H_ */
