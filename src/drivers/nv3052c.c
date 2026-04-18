#include <nv3052c.h>
#include <gpio.h>
#include <ccu.h>
#include <log.h>
#include <axp228.h>

/*
//#define HDP   600//玻璃实际(480) 需两边各插黑60处理
//#define VDP   1280

//#define VBPD   6
//#define VFPD  16
//#define VSPW   2
//#define HBPD  44
//#define HFPD  46 
//#define HSPW  2 

//frame rate=60Hz
//Pixel Clk=54.33Mhz


SPI_RES=1;
Delay(10);	//延迟10ms
SPI_RES=0;
Delay(220);	//延迟220ms
SPI_RES=1;
Delay(120);  //延迟120ms          

Delay(20);  //延迟20ms  
送平台数据



LCD             RV_Dock_EXT_3517    GPIO Function

1   LEDK    -   pd22_rgn_bl (5v)
2   LEDA
3   GND
4   VCC     -   out 3.3V
5   NC      -       (gnd)
6   NC      -       (gnd)
7   NC      -   pd12        -   LCD0-D18
8   R3          .
9   R4          .
10  R5          .
11  R6          .
12  R7      -   pd17        -   LCD0-D23
13  NC      -       (gnd)
14  NC      -       (gnd)
15  G2      -   pd6         -   LCD0-D10
16  G3          .
17  G4          .
18  G5          .
19  G6          .
20  G7      -   pd11        -   LCD0-D15
21  NC      -       (gnd)
22  NC      -       (gnd)
23  NC      -   pd0         -   LCD0-D2
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
37  SCL     -   pg13
38  CS      -   pe12
39  RESET   -   pe15       -   RESET
40  SDA     -   pe16

*/

timing_t timing = {
    .lcd_type = RGB,
	.pixclk = 54330000,//54.23Mhz
    .lcd_w = 600,      //width 
	.lcd_h = 1280,       //hight
    .scale_w = 480,
    .scale_h = 1280,

	.hbp = 44,          //horizontal back porch
	.ht = 600+44+46+2,  //horizontal Total Size 572
	.hspw = 2,          //horizontal Sync Pulse Width

	.vbp = 6,           //vertical back porch
	.vt = 1280+6+16+2,  //vertical Total Size
	.vspw = 2,          //vertical Sync Pulse Width
};

//dotclk = fframe × (X + HBP + HFP + HSPW) × (Y + VBP + VFP + VSPW) 54,225,120

struct gpio_t lcd_gpio[] = {

#if 1
	{
		.gpio = GPIOE,
		.pin = BV(15),  //RST
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
#else
	{
		.gpio = GPIOD,
		.pin =  BV(10),  //RST 19
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
		.pin =  0x3fffff, // D0-D9
		.mode = GPIO_MODE_FNC2,
		.pupd = GPIO_PUPD_OFF,
		.drv =  GPIO_DRV_3, 
		.state = GPIO_RESET,
	},
#endif
};

#define panel_reset_1 gpio_set(&lcd_gpio[0], GPIO_SET)
#define panel_reset_0 gpio_set(&lcd_gpio[0], GPIO_RESET)

#define panel_bl_1 gpio_set(&lcd_gpio[1], GPIO_SET)
#define panel_bl_0 gpio_set(&lcd_gpio[1], GPIO_RESET)

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
 	delay_ms(220);
	panel_reset_1;
 	delay_ms(120);

    LOG_D("=====================LCD_panel_init finish\n");
	return;
}



