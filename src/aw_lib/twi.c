#include "platform.h"
#include "gpio.h"
#include "ccu.h"
#include "log.h"
#include "twi.h"
#include "irq.h"

struct gpio_t twi_gpio[] = {
	{ // sck
		.gpio = GPIOB,
		.pin = BV(10),
		.mode = GPIO_MODE_FNC4,
		.pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
		//.state = GPIO_SET
	},
	{ // sda
		.gpio = GPIOB,
		.pin = BV(11),
		.mode = GPIO_MODE_FNC4,
		.pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
		//.state = GPIO_SET
	},
};

enum {
	I2C_STAT_BUS_ERROR	= 0x00,
	I2C_STAT_TX_START	= 0x08,
	I2C_STAT_TX_RSTART	= 0x10,
	I2C_STAT_TX_AW_ACK	= 0x18,
	I2C_STAT_TX_AW_NAK	= 0x20,
	I2C_STAT_TXD_ACK	= 0x28,
	I2C_STAT_TXD_NAK	= 0x30,
	I2C_STAT_LOST_ARB	= 0x38,
	I2C_STAT_TX_AR_ACK	= 0x40,
	I2C_STAT_TX_AR_NAK	= 0x48,
	I2C_STAT_RXD_ACK	= 0x50,
	I2C_STAT_RXD_NAK	= 0x58,
	I2C_STAT_IDLE		= 0xf8,
};

enum {
	I2C_M_TEN			= 0x0010,
	I2C_M_RD			= 0x0001,
	I2C_M_STOP			= 0x8000,
	I2C_M_NOSTART		= 0x4000,
	I2C_M_REV_DIR_ADDR	= 0x2000,
	I2C_M_IGNORE_NAK	= 0x1000,
	I2C_M_NO_RD_ACK		= 0x0800,
	I2C_M_RECV_LEN		= 0x0400,
};

struct i2c_msg_t {
	uint8_t addr;
	uint8_t flags;
	uint8_t len;
	uint8_t * buf;
};



/*
* Fin is APB CLOCK INPUT;
* Fsample = F0 = Fin/2^CLK_N;
* F1 = F0/(CLK_M+1);
* Foscl = F1/10 = Fin/(2^CLK_N * (CLK_M+1)*10);
* Foscl is clock SCL;100KHz or 400KHz
*
* clk_in: apb clk clock
* sclk_req: freqence to set in HZ
* For 400 kHz full speed 2-wire, CLK_N = 1, CLK_M = 2
*/

void twi_set_clock(TWI_TypeDef *twi, uint32_t tgtfreq)
{
	int apb1 = ccu_apb1_clk_get();
	int freq, delta, best_n = 7, best_m = 15;

	for (int n = 0; n <= 7; n++) {
		for (int m = 0; m <= 15; m++) {
			freq = apb1 / (10 * (m + 1) * (1 << n));
			delta = tgtfreq - freq;
			if (delta == 0)
			{
				best_n = n;
				best_m = m;
				break;
			}
		}
	}
	twi->TWI_CCR = (best_m << 3) | best_n;
}

static int d1_i2c_wait_status(TWI_TypeDef *twi)
{
	unsigned long timeout = get_time_ms() + 1;
	do {
		if(twi->TWI_CNTR & (1 << 3))
			return twi->TWI_STAT;
	} while(get_time_ms() < timeout);
	return I2C_STAT_BUS_ERROR;
}

static int d1_i2c_start(TWI_TypeDef *twi)
{
	twi->TWI_CNTR |= (1 << 5) | (1 << 3);

	unsigned long timeout = get_time_ms() + 1;

	do {
		if(!(twi->TWI_CNTR & (1 << 5)))
			break;
	} while(get_time_ms() < timeout);

	return d1_i2c_wait_status(twi);
}

static int d1_i2c_stop(TWI_TypeDef *twi)
{
	twi->TWI_CNTR |= (1 << 4) | (1 << 3);

	unsigned long  timeout = get_time_ms() + 1;

	do {
		if(!(twi->TWI_CNTR & (1 << 4)))
			break;
	} while(get_time_ms() < timeout);

	return d1_i2c_wait_status(twi);
}

static int d1_i2c_send_data(TWI_TypeDef *twi, uint8_t data)
{
	twi->TWI_DATA = data;
	twi->TWI_CNTR |= (1 << 3);
	return d1_i2c_wait_status(twi);
}

static int d1_i2c_read(TWI_TypeDef *twi, struct i2c_msg_t * msg)
{
	uint8_t * p = msg->buf;
	int len = msg->len;

	if(d1_i2c_send_data(twi, (uint8_t)(msg->addr << 1 | 1)) != I2C_STAT_TX_AR_ACK)
		return -1;

	twi->TWI_CNTR |= (1 << 2);
	while(len > 0)
	{
		if(len == 1)
		{
			twi->TWI_CNTR = (twi->TWI_CNTR & ~(1 << 2)) | (1 << 3);
			if(d1_i2c_wait_status(twi) != I2C_STAT_RXD_NAK)
				return -1;
		}
		else
		{
			twi->TWI_CNTR |= (1 << 3);
			if(d1_i2c_wait_status(twi) != I2C_STAT_RXD_ACK)
				return -1;
		}
		*p++ = twi->TWI_DATA;
		len--;
	}
	return 0;
}

static int d1_i2c_write(TWI_TypeDef *twi, struct i2c_msg_t * msg)
{
	uint8_t * p = msg->buf;
	int len = msg->len;

	if(d1_i2c_send_data(twi, (uint8_t)(msg->addr << 1)) != I2C_STAT_TX_AW_ACK)
		return -1;
	while(len > 0)
	{
		if(d1_i2c_send_data(twi, *p++) != I2C_STAT_TXD_ACK)
			return -1;
		len--;
	}
	return 0;
}

static int i2c_d1_xfer(TWI_TypeDef *twi, struct i2c_msg_t * msgs, int num)
{
	struct i2c_msg_t * pmsg = msgs;
	int i, res;

	if(!msgs || num <= 0)
		return 0;

	if(d1_i2c_start(twi) != I2C_STAT_TX_START)
		return 0;

	for(i = 0; i < num; i++, pmsg++)
	{
		if(i != 0)
		{
			if(d1_i2c_start(twi) != I2C_STAT_TX_RSTART)
				break;
		}
		if(pmsg->flags & I2C_M_RD)
			res = d1_i2c_read(twi, pmsg);
		else
			res = d1_i2c_write(twi, pmsg);
		if(res < 0)
			break;
	}
	d1_i2c_stop(twi);

	return i;
}

int i2c_master_send(TWI_TypeDef *twi,  struct i2c_msg_t * msg)
{
	msg->flags &= I2C_M_TEN;
	return i2c_d1_xfer(twi, msg, 1);
}

int i2c_master_recv(TWI_TypeDef *twi, struct i2c_msg_t * msg)
{
	msg->flags &= I2C_M_TEN;
	msg->flags |= I2C_M_RD;
	return i2c_d1_xfer(twi, msg, 1);
}

#define AXP228_ADDR (0x69 >> 1)
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



void _axp_get_reg(TWI_TypeDef *twi, uint8_t reg_addr)
{
	struct i2c_msg_t msg[2];

    msg[0].addr = AXP228_ADDR;
    msg[0].buf = &reg_addr;
	msg[0].flags = 0;
    msg[0].len = sizeof(reg_addr);

    msg[1].addr = AXP228_ADDR;
    msg[1].buf = &reg_addr;
	msg[1].flags |= I2C_M_RD;
    msg[1].len = sizeof(reg_addr);

	i2c_d1_xfer(twi, &msg[0], 2);

	uint8_t value = *msg[1].buf;
    LOG_D("REG: %d", value);
}	

void twi_int_handler(void)
{
    LOG_D("twi_int_handler");
}

void twi_init(void)
{
	LOG_D("twi: init\n\r");
	gpio_init(twi_gpio, ARRAY_SIZE(twi_gpio));
	ccu_twi_enable(TWI0);
	twi_set_clock(TWI0, 400000);
	TWI0->TWI_CNTR = (1 << 6);
	TWI0->TWI_SRST = (1 << 0);

	irq_assign(TWI0_IRQn, (void *) twi_int_handler);
	irq_enable(TWI0_IRQn);

 	_axp_get_reg(TWI0, DLDO1_Voltage_REG);
	_axp_get_reg(TWI0, DLDO2_Voltage_REG);
	_axp_get_reg(TWI0, DLDO3_Voltage_REG);
	_axp_get_reg(TWI0, DLDO4_Voltage_REG);
	_axp_get_reg(TWI0, ELDO1_Voltage_REG);
	_axp_get_reg(TWI0, ELDO2_Voltage_REG);
	_axp_get_reg(TWI0, ELDO3_Voltage_REG);
	_axp_get_reg(TWI0, DC5LD_Voltage_REG);
	_axp_get_reg(TWI0, DCDC1_Voltage_REG);
	_axp_get_reg(TWI0, DCDC2_Voltage_REG);
	_axp_get_reg(TWI0, DCDC3_Voltage_REG);
	_axp_get_reg(TWI0, DCDC4_Voltage_REG);
	_axp_get_reg(TWI0, DCDC5_Voltage_REG);
	_axp_get_reg(TWI0, ALDO1_Voltage_REG);
	_axp_get_reg(TWI0, ALDO2_Voltage_REG);
	_axp_get_reg(TWI0, ALDO3_Voltage_REG);


}
