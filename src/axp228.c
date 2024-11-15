#include "platform.h"
#include "log.h"
#include "twi.h"

#define AXP228_ADDR (0x69 >> 1)

#define DCDC_CNTRL_REG 0x10
#define ELDO_CNTRL_REG 0x12
#define ALDO_CNTRL_REG 0x13

#define DLDO1_Voltage_REG 0x15
#define DLDO2_Voltage_REG 0x16
#define DLDO3_Voltage_REG 0x17
#define DLDO4_Voltage_REG 0x18
#define ELDO1_Voltage_REG 0x19
#define ELDO2_Voltage_REG 0x1A
#define ELDO3_Voltage_REG 0x1B
#define DC5LD_Voltage_REG 0x1C
#define DCDC1_Voltage_REG 0x21
#define DCDC2_Voltage_REG 0x22
#define DCDC3_Voltage_REG 0x23
#define DCDC4_Voltage_REG 0x24
#define DCDC5_Voltage_REG 0x25
#define ALDO1_Voltage_REG 0x28
#define ALDO2_Voltage_REG 0x29
#define ALDO3_Voltage_REG 0x2A

//DCDC_CNTRL_REG
#define ALDO2_Enable BV(7)
#define ALDO1_Enable BV(6)
#define DCDC5_Enable BV(5)
#define DCDC4_Enable BV(4)
#define DCDC3_Enable BV(3)
#define DCDC2_Enable BV(2)
#define DCDC1_Enable BV(1)
#define DC5LD_Enable BV(0)

//ELDO_CNTRL_REG
#define DC1SW_Enable BV(7)
#define DLDO4_Enable BV(6)
#define DLDO3_Enable BV(5)
#define DLDO2_Enable BV(4)
#define DLDO1_Enable BV(3)
#define ELDO3_Enable BV(2)
#define ELDO2_Enable BV(1)
#define ELDO1_Enable BV(0)

//ALDO_CNTRL_REG
#define ALDO3_Enable BV(7)

struct dcdc_reg
{
    char name[40];
    uint8_t addr;
    uint8_t value;
};

#define ENABLE_REGISTERS {\
            {"DCDC1-5 ALDO1-2 DC5LDO Control",DCDC_CNTRL_REG,0},\
            {"ELDO1-3 DLDO1-4 DC1SW Control ",ELDO_CNTRL_REG,0},\
            {"ADLDO3 Control                ",ALDO_CNTRL_REG,0},\
                         }  

#define VOLTAGE_REGISTERS {\
            {"DLDO1  Voltage Set",DLDO1_Voltage_REG,0},\
            {"DLDO2  Voltage Set",DLDO2_Voltage_REG,0},\
            {"DLDO3  Voltage Set",DLDO3_Voltage_REG,0},\
            {"DLDO4  Voltage Set",DLDO4_Voltage_REG,0},\
            {"ELDO1  Voltage Set",ELDO1_Voltage_REG,0},\
            {"ELDO2  Voltage Set",ELDO2_Voltage_REG,0},\
            {"ELDO3  Voltage Set",ELDO3_Voltage_REG,0},\
            {"DC5LDO Voltage Set",DC5LD_Voltage_REG,0},\
            {"DCDC1  Voltage Set",DCDC1_Voltage_REG,0},\
            {"DCDC2  Voltage Set",DCDC2_Voltage_REG,0},\
            {"DCDC3  Voltage Set",DCDC3_Voltage_REG,0},\
            {"DCDC4  Voltage Set",DCDC4_Voltage_REG,0},\
            {"DCDC5  Voltage Set",DCDC5_Voltage_REG,0},\
            {"ALDO1  Voltage Set",ALDO1_Voltage_REG,0},\
            {"ALDO2  Voltage Set",ALDO2_Voltage_REG,0},\
            {"ALDO3  Voltage Set",ALDO3_Voltage_REG,0},\
    };

void axp_set_reg(TWI_TypeDef *twi, uint8_t reg_addr, uint8_t reg_value)
{
    i2c_master_set(twi, AXP228_ADDR, reg_addr, reg_value);
}

uint8_t axp_get_reg(TWI_TypeDef *twi, uint8_t reg_addr)
{
    return i2c_master_get(twi, AXP228_ADDR, reg_addr);
}

static void axp_get_regsisters(TWI_TypeDef *twi, struct dcdc_reg * dcdc_regs, int dcdc_regs_len)
{
    for (int i =0; i < dcdc_regs_len; i++)
    {
        dcdc_regs[i].value = axp_get_reg(twi, dcdc_regs[i].addr);
    }
}

void axp_get_voltage(TWI_TypeDef *twi)
{
    struct dcdc_reg dcdc_regs[] = VOLTAGE_REGISTERS;

    axp_get_regsisters(twi, dcdc_regs,ARRAY_SIZE(dcdc_regs));

    for (int i =0; i < ARRAY_SIZE(dcdc_regs); i++)
    {
        LOG_D("Reg name:'%s' addr: %x value: %d",
        dcdc_regs[i].name, 
        dcdc_regs[i].addr, 
        dcdc_regs[i].value);
    }
}

void axp_get_enable(TWI_TypeDef *twi)
{
    struct dcdc_reg dcdc_regs[] = ENABLE_REGISTERS;

    axp_get_regsisters(twi, dcdc_regs,ARRAY_SIZE(dcdc_regs));

    for (int i =0; i < ARRAY_SIZE(dcdc_regs); i++)
    {
        LOG_D("Reg name:'%s' addr: %x value: %x",
        dcdc_regs[i].name, 
        dcdc_regs[i].addr, 
        dcdc_regs[i].value);

        if (dcdc_regs[i].addr == DCDC_CNTRL_REG)
        {
            LOG_D("    ALDO2_Enable %d", dcdc_regs[i].value & ALDO2_Enable );
            LOG_D("    ALDO1_Enable %d", dcdc_regs[i].value & ALDO1_Enable );
            LOG_D("    DCDC5_Enable %d", dcdc_regs[i].value & DCDC5_Enable );
            LOG_D("    DCDC4_Enable %d", dcdc_regs[i].value & DCDC4_Enable );
            LOG_D("    DCDC3_Enable %d", dcdc_regs[i].value & DCDC3_Enable );
            LOG_D("    DCDC2_Enable %d", dcdc_regs[i].value & DCDC2_Enable );
            LOG_D("    DCDC1_Enable %d", dcdc_regs[i].value & DCDC1_Enable );
            LOG_D("    DC5LD_Enable %d", dcdc_regs[i].value & DC5LD_Enable );
        }
        
        if (dcdc_regs[i].addr == ELDO_CNTRL_REG)
        {
            LOG_D("    DC1SW_Enable %d", dcdc_regs[i].value & DC1SW_Enable );
            LOG_D("    DLDO4_Enable %d", dcdc_regs[i].value & DLDO4_Enable );
            LOG_D("    DLDO3_Enable %d", dcdc_regs[i].value & DLDO3_Enable );
            LOG_D("    DLDO2_Enable %d", dcdc_regs[i].value & DLDO2_Enable );
            LOG_D("    DLDO1_Enable %d", dcdc_regs[i].value & DLDO1_Enable );
            LOG_D("    ELDO3_Enable %d", dcdc_regs[i].value & ELDO3_Enable );
            LOG_D("    ELDO2_Enable %d", dcdc_regs[i].value & ELDO2_Enable );
            LOG_D("    ELDO1_Enable %d", dcdc_regs[i].value & ELDO1_Enable );
        }
        if (dcdc_regs[i].addr == ALDO_CNTRL_REG)
        {
            LOG_D("    ALDO3_Enable %d", dcdc_regs[i].value & ALDO3_Enable );
        }
    }
}


void axp_LCD_control(TWI_TypeDef *twi, int on_off)
{
    if (on_off)
        axp_set_reg(twi, DCDC_CNTRL_REG, axp_get_reg(twi, DCDC_CNTRL_REG) |  (DCDC3_Enable | ALDO2_Enable));
    else
        axp_set_reg(twi, DCDC_CNTRL_REG, axp_get_reg(twi, DCDC_CNTRL_REG) & ~(DCDC3_Enable | ALDO2_Enable));
}

void axp_USB_control(TWI_TypeDef *twi, int on_off)
{
    if (on_off)
    {
        axp_set_reg(twi, DCDC_CNTRL_REG,     axp_get_reg(twi, DCDC_CNTRL_REG)     |  DCDC1_Enable );
        axp_set_reg(twi, DLDO2_Voltage_REG,  axp_get_reg(twi, DLDO2_Voltage_REG)  |  0x1f );
        axp_set_reg(twi, ELDO_CNTRL_REG,     axp_get_reg(twi, ELDO_CNTRL_REG)     |  DLDO2_Enable );
    }
    else
    {
        axp_set_reg(twi, ELDO_CNTRL_REG,     axp_get_reg(twi, ELDO_CNTRL_REG)     & ~DLDO2_Enable );
        axp_set_reg(twi, DLDO2_Voltage_REG,  axp_get_reg(twi, DLDO2_Voltage_REG)  &  0x00 );
    }    
}