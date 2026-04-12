#include <nv3052c.h>
#include <gpio.h>
#include <ccu.h>
#include <log.h>
#include <axp228.h>

/*
LCD             RV_Dock_EXT_3517    GPIO Function

1   LEDK    -   pd22_rgn_bl (5v)
2   LEDA
3   GND
4   VCC     -   out 3.3V
5   R0      -       (gnd)
6   R1      -       (gnd)
7   R2      -   pd12        -   LCD0-D18
8   R3          .
9   R4          .
10  R5          .
11  R6          .
12  R7      -   pd17        -   LCD0-D23
13  G0      -       (gnd)
14  G1      -       (gnd)
15  G2      -   pd6         -   LCD0-D10
16  G3          .
17  G4          .
18  G5          .
19  G6          .
20  G7      -   pd11        -   LCD0-D15
21  B0      -       (gnd)
22  B1      -       (gnd)
23  B2      -   pd0         -   LCD0-D2
24  B3          .
25  B4          .
26  B5          .
27  B6          .
28  B7      -   pd5         -   LCD0-D7
29  GND
30  CLK     -   pd18        -   LCD0-CLK
31  DISP    -   (NC)
32  Hsync   -   pd20        -   LCD0-HSYNK
33  Vsync   -   pd21        -   LCD0-VSYNK
34  DEN     -   pd19        -   LCD0-DE
35  NC      -   (NC)
36  GND
37  XR      -   pg13        -   Reset ?
38  YD      -   pe12
39  XL      -   pe15
40  YU      -   pe16

*/

timing_t timing = {
    .lcd_type = RGB,
	.pixclk = 55000000,//54.23Mhz
    .lcd_w = 600,      //width 
	.lcd_h = 1280,       //hight

	.hbp = 8,          //horizontal back porch
	.ht = 600+8+8+2,  //horizontal Total Size 572
	.hspw = 2,          //horizontal Sync Pulse Width

	.vbp = 8,           //vertical back porch
	.vt = 1280+8+8+2,  //vertical Total Size
	.vspw = 2,          //vertical Sync Pulse Width
};

//dotclk = fframe × (X + HBP + HFP + HSPW) × (Y + VBP + VFP + VSPW) 54,225,120



struct gpio_t lcd_gpio[] = {

#if 1
	{
		.gpio = GPIOG,
		.pin = BV(13),  //RST
		.mode = GPIO_MODE_OUTPUT,
        .pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
	},
    {
		.gpio = GPIOD,
		.pin =  BV(22), //BL
		.pupd = GPIO_PUPD_UP,
		.mode = GPIO_MODE_OUTPUT,
		.drv =  GPIO_DRV_3,
	},
	{
		.gpio = GPIOD,
		.pin =  0x3fffff, // D0-D21
		.mode = GPIO_MODE_FNC2,
		.pupd = GPIO_PUPD_OFF,
		.drv =  GPIO_DRV_3, 
		.state = GPIO_RESET,
	},
    {
		.gpio = GPIOE,
		.pin = BV(14),  //CS
		.mode = GPIO_MODE_OUTPUT,
        .pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
	},
    {
		.gpio = GPIOE,
		.pin = BV(12),  //SDI
		.mode = GPIO_MODE_OUTPUT,
        .pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
	},
    {
		.gpio = GPIOE,
		.pin = BV(15),  //SCL
		.mode = GPIO_MODE_OUTPUT,
        .pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
	},
#else
	{
		.gpio = GPIOD,
		.pin =  BV(19),  //RST
		.mode = GPIO_MODE_OUTPUT,
        .pupd = GPIO_PUPD_DOWN,
		.drv =  GPIO_DRV_3,
        .state = GPIO_SET,
	},


    {
		.gpio = GPIOD,
		.pin =  BV(20), //BL 22
		.pupd = GPIO_PUPD_UP,
		.mode = GPIO_MODE_OUTPUT,
		.drv =  GPIO_DRV_3,
	},
	{
		.gpio = GPIOD,
		.pin =  0x3ff, // D0-D9
		.mode = GPIO_MODE_FNC3,
		.pupd = GPIO_PUPD_OFF,
		.drv =  GPIO_DRV_3, 
		.state = GPIO_RESET,
	},
    
    {
		.gpio = GPIOD,
		.pin = BV(10),  //CS
		.mode = GPIO_MODE_FNC4,
        .pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
	},
    {
		.gpio = GPIOD,
		.pin = BV(11),  //SCL
		.mode = GPIO_MODE_FNC4,
        .pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
	},
    {
		.gpio = GPIOD,
		.pin = BV(12),  //SDA
		.mode = GPIO_MODE_FNC4,
        .pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
	},
    {
		.gpio = GPIOD,
		.pin = BV(13),  //SDO
		.mode = GPIO_MODE_FNC4,
        .pupd = GPIO_PUPD_UP,
		.drv = GPIO_DRV_3,
	}, 
#endif
};



struct nv3052c_reg {
	uint8_t cmd;
	uint8_t val;
};

#define panel_reset_1 gpio_set(&lcd_gpio[0], GPIO_SET)
#define panel_reset_0 gpio_set(&lcd_gpio[0], GPIO_RESET)

#define panel_bl_1 gpio_set(&lcd_gpio[1], GPIO_SET)
#define panel_bl_0 gpio_set(&lcd_gpio[1], GPIO_RESET)

#define panel_spi_cs_1    gpio_set(&lcd_gpio[3], GPIO_SET)
#define panel_spi_cs_0    gpio_set(&lcd_gpio[3], GPIO_RESET)

#define panel_spi_sdi_1   gpio_set(&lcd_gpio[4], GPIO_SET)
#define panel_spi_sdi_0   gpio_set(&lcd_gpio[4], GPIO_RESET)

#define panel_spi_scl_1   gpio_set(&lcd_gpio[5], GPIO_SET)
#define panel_spi_scl_0   gpio_set(&lcd_gpio[5], GPIO_RESET)

/*
 * Common initialization registers for all currently
 * supported displays. Mostly seem to be related
 * to Gamma correction curves and output pad mappings.
 */
static const struct nv3052c_reg common_init_regs[] = {
	// EXTC Command set enable, select page 2
	{ 0xff, 0x30 }, { 0xff, 0x52 }, { 0xff, 0x02 },
	// Set gray scale voltage to adjust gamma
	{ 0xb0, 0x0b }, // PGAMVR0
	{ 0xb1, 0x16 }, // PGAMVR1
	{ 0xb2, 0x17 }, // PGAMVR2
	{ 0xb3, 0x2c }, // PGAMVR3
	{ 0xb4, 0x32 }, // PGAMVR4
	{ 0xb5, 0x3b }, // PGAMVR5
	{ 0xb6, 0x29 }, // PGAMPR0
	{ 0xb7, 0x40 }, // PGAMPR1
	{ 0xb8, 0x0d }, // PGAMPK0
	{ 0xb9, 0x05 }, // PGAMPK1
	{ 0xba, 0x12 }, // PGAMPK2
	{ 0xbb, 0x10 }, // PGAMPK3
	{ 0xbc, 0x12 }, // PGAMPK4
	{ 0xbd, 0x15 }, // PGAMPK5
	{ 0xbe, 0x19 }, // PGAMPK6
	{ 0xbf, 0x0e }, // PGAMPK7
	{ 0xc0, 0x16 }, // PGAMPK8
	{ 0xc1, 0x0a }, // PGAMPK9
	// Set gray scale voltage to adjust gamma
	{ 0xd0, 0x0c }, // NGAMVR0
	{ 0xd1, 0x17 }, // NGAMVR0
	{ 0xd2, 0x14 }, // NGAMVR1
	{ 0xd3, 0x2e }, // NGAMVR2
	{ 0xd4, 0x32 }, // NGAMVR3
	{ 0xd5, 0x3c }, // NGAMVR4
	{ 0xd6, 0x22 }, // NGAMPR0
	{ 0xd7, 0x3d }, // NGAMPR1
	{ 0xd8, 0x0d }, // NGAMPK0
	{ 0xd9, 0x07 }, // NGAMPK1
	{ 0xda, 0x13 }, // NGAMPK2
	{ 0xdb, 0x13 }, // NGAMPK3
	{ 0xdc, 0x11 }, // NGAMPK4
	{ 0xdd, 0x15 }, // NGAMPK5
	{ 0xde, 0x19 }, // NGAMPK6
	{ 0xdf, 0x10 }, // NGAMPK7
	{ 0xe0, 0x17 }, // NGAMPK8
	{ 0xe1, 0x0a }, // NGAMPK9
	// EXTC Command set enable, select page 3
	{ 0xff, 0x30 }, { 0xff, 0x52 }, { 0xff, 0x03 },
	// Set various timing settings
	{ 0x00, 0x2a }, // GIP_VST_1
	{ 0x01, 0x2a }, // GIP_VST_2
	{ 0x02, 0x2a }, // GIP_VST_3
	{ 0x03, 0x2a }, // GIP_VST_4
	{ 0x04, 0x61 }, // GIP_VST_5
	{ 0x05, 0x80 }, // GIP_VST_6
	{ 0x06, 0xc7 }, // GIP_VST_7
	{ 0x07, 0x01 }, // GIP_VST_8
	{ 0x08, 0x03 }, // GIP_VST_9
	{ 0x09, 0x04 }, // GIP_VST_10
	{ 0x70, 0x22 }, // GIP_ECLK1
	{ 0x71, 0x80 }, // GIP_ECLK2
	{ 0x30, 0x2a }, // GIP_CLK_1
	{ 0x31, 0x2a }, // GIP_CLK_2
	{ 0x32, 0x2a }, // GIP_CLK_3
	{ 0x33, 0x2a }, // GIP_CLK_4
	{ 0x34, 0x61 }, // GIP_CLK_5
	{ 0x35, 0xc5 }, // GIP_CLK_6
	{ 0x36, 0x80 }, // GIP_CLK_7
	{ 0x37, 0x23 }, // GIP_CLK_8
	{ 0x40, 0x03 }, // GIP_CLKA_1
	{ 0x41, 0x04 }, // GIP_CLKA_2
	{ 0x42, 0x05 }, // GIP_CLKA_3
	{ 0x43, 0x06 }, // GIP_CLKA_4
	{ 0x44, 0x11 }, // GIP_CLKA_5
	{ 0x45, 0xe8 }, // GIP_CLKA_6
	{ 0x46, 0xe9 }, // GIP_CLKA_7
	{ 0x47, 0x11 }, // GIP_CLKA_8
	{ 0x48, 0xea }, // GIP_CLKA_9
	{ 0x49, 0xeb }, // GIP_CLKA_10
	{ 0x50, 0x07 }, // GIP_CLKB_1
	{ 0x51, 0x08 }, // GIP_CLKB_2
	{ 0x52, 0x09 }, // GIP_CLKB_3
	{ 0x53, 0x0a }, // GIP_CLKB_4
	{ 0x54, 0x11 }, // GIP_CLKB_5
	{ 0x55, 0xec }, // GIP_CLKB_6
	{ 0x56, 0xed }, // GIP_CLKB_7
	{ 0x57, 0x11 }, // GIP_CLKB_8
	{ 0x58, 0xef }, // GIP_CLKB_9
	{ 0x59, 0xf0 }, // GIP_CLKB_10
	// Map internal GOA signals to GOA output pad
	{ 0xb1, 0x01 }, // PANELD2U2
	{ 0xb4, 0x15 }, // PANELD2U5
	{ 0xb5, 0x16 }, // PANELD2U6
	{ 0xb6, 0x09 }, // PANELD2U7
	{ 0xb7, 0x0f }, // PANELD2U8
	{ 0xb8, 0x0d }, // PANELD2U9
	{ 0xb9, 0x0b }, // PANELD2U10
	{ 0xba, 0x00 }, // PANELD2U11
	{ 0xc7, 0x02 }, // PANELD2U24
	{ 0xca, 0x17 }, // PANELD2U27
	{ 0xcb, 0x18 }, // PANELD2U28
	{ 0xcc, 0x0a }, // PANELD2U29
	{ 0xcd, 0x10 }, // PANELD2U30
	{ 0xce, 0x0e }, // PANELD2U31
	{ 0xcf, 0x0c }, // PANELD2U32
	{ 0xd0, 0x00 }, // PANELD2U33
	// Map internal GOA signals to GOA output pad
	{ 0x81, 0x00 }, // PANELU2D2
	{ 0x84, 0x15 }, // PANELU2D5
	{ 0x85, 0x16 }, // PANELU2D6
	{ 0x86, 0x10 }, // PANELU2D7
	{ 0x87, 0x0a }, // PANELU2D8
	{ 0x88, 0x0c }, // PANELU2D9
	{ 0x89, 0x0e }, // PANELU2D10
	{ 0x8a, 0x02 }, // PANELU2D11
	{ 0x97, 0x00 }, // PANELU2D24
	{ 0x9a, 0x17 }, // PANELU2D27
	{ 0x9b, 0x18 }, // PANELU2D28
	{ 0x9c, 0x0f }, // PANELU2D29
	{ 0x9d, 0x09 }, // PANELU2D30
	{ 0x9e, 0x0b }, // PANELU2D31
	{ 0x9f, 0x0d }, // PANELU2D32
	{ 0xa0, 0x01 }, // PANELU2D33
	// EXTC Command set enable, select page 2
	{ 0xff, 0x30 }, { 0xff, 0x52 }, { 0xff, 0x02 },
	// Page 2 register values (0x01..0x10) are same for nv3051d and nv3052c
	{ 0x01, 0x01 },
	{ 0x02, 0xda },
	{ 0x03, 0xba },
	{ 0x04, 0xa8 },
	{ 0x05, 0x9a },
	{ 0x06, 0x70 },
	{ 0x07, 0xff },
	{ 0x08, 0x91 },
	{ 0x09, 0x90 },
	{ 0x0a, 0xff },
	{ 0x0b, 0x8f },
	{ 0x0c, 0x60 },
	{ 0x0d, 0x58 },
	{ 0x0e, 0x48 },
	{ 0x0f, 0x38 },
	{ 0x10, 0x2b },
	// EXTC Command set enable, select page 0
	{ 0xff, 0x30 }, { 0xff, 0x52 }, { 0xff, 0x00 },
	// Display Access Control
	{ 0x36, 0x0a }, // bgr = 1, ss = 1, gs = 0

};


//three line 9bit mode
static void LCD_WRITE_DATA(uint32_t value)
{
	uint32_t i;
	panel_spi_cs_0; //14
	panel_spi_sdi_1; //12
	panel_spi_scl_0; //15
	//!sunxi_lcd_delay_us(10);
	panel_spi_scl_1; //15
	for (i = 0; i < 8; i++) {
		//!sunxi_lcd_delay_us(10);
		if (value & (0x80 >> i))
			panel_spi_sdi_1; //12
		else
			panel_spi_sdi_0;//12
		//!value <<= 1;
		//!sunxi_lcd_delay_us(10);
		panel_spi_scl_0;//15
		panel_spi_scl_1;//15
	}
	//!sunxi_lcd_delay_us(10);
	panel_spi_cs_1;//14
}

static void LCD_WRITE_COMMAND(uint32_t value)
{
	uint32_t i;
	panel_spi_cs_0; //14
	panel_spi_sdi_0; //12
	panel_spi_scl_0; //15
	//!sunxi_lcd_delay_us(10);
	panel_spi_scl_1; //15
	for (i = 0; i < 8; i++) {
		//!sunxi_lcd_delay_us(10);
		if (value & (0x80 >> i))
			panel_spi_sdi_1; //12
		else
			panel_spi_sdi_0; //12
		panel_spi_scl_0; //15
		//!sunxi_lcd_delay_us(10);
		panel_spi_scl_1; //15
		//!value <<= 1;
	}
	//!sunxi_lcd_delay_us(10);
	panel_spi_cs_1; //14
}


timing_t LCD_get_param(void)
{
	return timing;
}

void LCD_gpio_init(void)
{
    gpio_init(lcd_gpio, ARRAY_SIZE(lcd_gpio));
}

void LCD_bl_open(void)
{
	panel_bl_1;
}

void LCD_bl_close(void)
{
	panel_bl_0;
}

void LCD_panel_init(void)
{
    LOG_D("=====================LCD_panel_init 3052\n");
	axp_LCD_control(TWI0,1);
    panel_reset_1;
	delay_ms(10);
    panel_reset_0;
	delay_ms(130);
    //panel_reset_1;
	//delay_ms(120);

 	panel_spi_cs_0; //14
	panel_spi_sdi_1; //12
	panel_spi_scl_0; //15
	panel_spi_scl_1; //15
 
 /*    for (int i = 0; i < ARRAY_SIZE(common_init_regs), i++;)
    {
        LCD_WRITE_COMMAND(common_init_regs[i].cmd);
        LCD_WRITE_DATA(common_init_regs[i].val);
    }  */

    LOG_D("=====================LCD_panel_init finish\n");
	return;
}



