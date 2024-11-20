/*
 * drivers/video/fbdev/sunxi/disp2/disp/lcd/st7701s_rgb.c
 *
 * Copyright (c) 2018-2021 Allwinnertech Co., Ltd.
 * Author: zepan <zepan@sipeed.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <st7701s_rgb.h>
#include <gpio.h>
#include <ccu.h>
#include <log.h>

//https://github.com/Tina-Linux/LCD_Panel_Driver/tree/master/LCD/BH040I-01_ST7701s_RGB_480x480

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
	.pixclk = 12000000,
	.lcd_w = 480,
	.lcd_h = 272,
	.hbp = 60,
	.ht = 612,
	.hspw = 12,
	.vbp = 18,
	.vt = 520,
	.vspw = 4,
};

struct gpio_t lcd_gpio[] = {
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
};

#define st7701s_spi_reset_1 gpio_set(&lcd_gpio[0], GPIO_SET)
#define st7701s_spi_reset_0 gpio_set(&lcd_gpio[0], GPIO_RESET)

#define panel_bl_1 gpio_set(&lcd_gpio[1], GPIO_SET)
#define panel_bl_0 gpio_set(&lcd_gpio[1], GPIO_RESET)

#define st7701s_spi_cs_1    gpio_set(&lcd_gpio[3], GPIO_SET)
#define st7701s_spi_cs_0    gpio_set(&lcd_gpio[3], GPIO_RESET)

#define st7701s_spi_sdi_1   gpio_set(&lcd_gpio[4], GPIO_SET)
#define st7701s_spi_sdi_0   gpio_set(&lcd_gpio[4], GPIO_RESET)

#define st7701s_spi_scl_1   gpio_set(&lcd_gpio[5], GPIO_SET)
#define st7701s_spi_scl_0   gpio_set(&lcd_gpio[5], GPIO_RESET)

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


//three line 9bit mode
static void LCD_WRITE_DATA(uint32_t value)
{
	uint32_t i;
	st7701s_spi_cs_0; //14
	st7701s_spi_sdi_1; //12
	st7701s_spi_scl_0; //15
	//!sunxi_lcd_delay_us(10);
	st7701s_spi_scl_1; //15
	for (i = 0; i < 8; i++) {
		//!sunxi_lcd_delay_us(10);
		if (value & (0x80 >> i))
			st7701s_spi_sdi_1; //12
		else
			st7701s_spi_sdi_0;//12
		//!value <<= 1;
		//!sunxi_lcd_delay_us(10);
		st7701s_spi_scl_0;//15
		st7701s_spi_scl_1;//15
	}
	//!sunxi_lcd_delay_us(10);
	st7701s_spi_cs_1;//14
}

static void LCD_WRITE_COMMAND(uint32_t value)
{
	uint32_t i;
	st7701s_spi_cs_0; //14
	st7701s_spi_sdi_0; //12
	st7701s_spi_scl_0; //15
	//!sunxi_lcd_delay_us(10);
	st7701s_spi_scl_1; //15
	for (i = 0; i < 8; i++) {
		//!sunxi_lcd_delay_us(10);
		if (value & (0x80 >> i))
			st7701s_spi_sdi_1; //12
		else
			st7701s_spi_sdi_0; //12
		st7701s_spi_scl_0; //15
		//!sunxi_lcd_delay_us(10);
		st7701s_spi_scl_1; //15
		//!value <<= 1;
	}
	//!sunxi_lcd_delay_us(10);
	st7701s_spi_cs_1; //14
}

void LCD_panel_init(void)
{
    LOG_D("=====================LCD_panel_init\n");
    
    //gpio_init(lcd_gpio, ARRAY_SIZE(lcd_gpio));

    st7701s_spi_reset_0;
	delay_ms(10);
    st7701s_spi_reset_1;
	delay_ms(10);
    

    LCD_WRITE_COMMAND(0xFF);
    LCD_WRITE_DATA(0x77);
    LCD_WRITE_DATA(0x01);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x10);

    LCD_WRITE_COMMAND(0xC0);
    LCD_WRITE_DATA(0x3B);
    LCD_WRITE_DATA(0x00);

    LCD_WRITE_COMMAND(0xC1);
    LCD_WRITE_DATA(0x0D);
    LCD_WRITE_DATA(0x02);

    LCD_WRITE_COMMAND(0xC2);
    LCD_WRITE_DATA(0x21);
    LCD_WRITE_DATA(0x08);

//    //RGB Interface Setting
//    LCD_WRITE_COMMAND(0xC3);
//    LCD_WRITE_DATA(0x02);
    
    LCD_WRITE_COMMAND(0xCD);
    LCD_WRITE_DATA(0x18);//0F 08-OK  D0-D18      
      

    LCD_WRITE_COMMAND(0xB0);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x18);
    LCD_WRITE_DATA(0x0E);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x06);
    LCD_WRITE_DATA(0x07);
    LCD_WRITE_DATA(0x08);
    LCD_WRITE_DATA(0x07);
    LCD_WRITE_DATA(0x22);
    LCD_WRITE_DATA(0x04);
    LCD_WRITE_DATA(0x12);
    LCD_WRITE_DATA(0x0F);
    LCD_WRITE_DATA(0xAA);
    LCD_WRITE_DATA(0x31);
    LCD_WRITE_DATA(0x18);

    LCD_WRITE_COMMAND(0xB1);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x19);
    LCD_WRITE_DATA(0x0E);
    LCD_WRITE_DATA(0x12);
    LCD_WRITE_DATA(0x07);
    LCD_WRITE_DATA(0x08);
    LCD_WRITE_DATA(0x08);
    LCD_WRITE_DATA(0x08);
    LCD_WRITE_DATA(0x22);
    LCD_WRITE_DATA(0x04);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0xA9);
    LCD_WRITE_DATA(0x32);
    LCD_WRITE_DATA(0x18);

    LCD_WRITE_COMMAND(0xFF);
    LCD_WRITE_DATA(0x77);
    LCD_WRITE_DATA(0x01);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x11);

    LCD_WRITE_COMMAND(0xB0);
    LCD_WRITE_DATA(0x60);

    LCD_WRITE_COMMAND(0xB1);
    LCD_WRITE_DATA(0x30);

    LCD_WRITE_COMMAND(0xB2);
    LCD_WRITE_DATA(0x87);

    LCD_WRITE_COMMAND(0xB3);
    LCD_WRITE_DATA(0x80);

    LCD_WRITE_COMMAND(0xB5);
    LCD_WRITE_DATA(0x49);

    LCD_WRITE_COMMAND(0xB7);
    LCD_WRITE_DATA(0x85);

    LCD_WRITE_COMMAND(0xB8);
    LCD_WRITE_DATA(0x21);

    LCD_WRITE_COMMAND(0xC1);
    LCD_WRITE_DATA(0x78);

    LCD_WRITE_COMMAND(0xC2);
    LCD_WRITE_DATA(0x78);

    delay_ms(10);

    LCD_WRITE_COMMAND(0xE0);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x1B);
    LCD_WRITE_DATA(0x02);

    LCD_WRITE_COMMAND(0xE1);
    LCD_WRITE_DATA(0x08);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x07);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x44);
    LCD_WRITE_DATA(0x44);

    LCD_WRITE_COMMAND(0xE2);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x44);
    LCD_WRITE_DATA(0x44);
    LCD_WRITE_DATA(0xED);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0xEC);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);

    LCD_WRITE_COMMAND(0xE3);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x11);

    LCD_WRITE_COMMAND(0xE4);
    LCD_WRITE_DATA(0x44);
    LCD_WRITE_DATA(0x44);

    LCD_WRITE_COMMAND(0xE5);
    LCD_WRITE_DATA(0x0A);
    LCD_WRITE_DATA(0xE9);
    LCD_WRITE_DATA(0xD8);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x0C);
    LCD_WRITE_DATA(0xEB);
    LCD_WRITE_DATA(0xD8);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x0E);
    LCD_WRITE_DATA(0xED);
    LCD_WRITE_DATA(0xD8);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x10);
    LCD_WRITE_DATA(0xEF);
    LCD_WRITE_DATA(0xD8);
    LCD_WRITE_DATA(0xA0);

    LCD_WRITE_COMMAND(0xE6);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x11);

    LCD_WRITE_COMMAND(0xE7);
    LCD_WRITE_DATA(0x44);
    LCD_WRITE_DATA(0x44);

    LCD_WRITE_COMMAND(0xE8);
    LCD_WRITE_DATA(0x09);
    LCD_WRITE_DATA(0xE8);
    LCD_WRITE_DATA(0xD8);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x0B);
    LCD_WRITE_DATA(0xEA);
    LCD_WRITE_DATA(0xD8);
    LCD_WRITE_DATA(0xA0);
    LCD_WRITE_DATA(0x0D);
    LCD_WRITE_DATA(0xEC);
    LCD_WRITE_DATA(0xD8);
    LCD_WRITE_DATA(0xA0); 
    LCD_WRITE_DATA(0x0F);
    LCD_WRITE_DATA(0xEE);
    LCD_WRITE_DATA(0xD8);
    LCD_WRITE_DATA(0xA0);

    LCD_WRITE_COMMAND(0xEB);
    LCD_WRITE_DATA(0x02);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0xE4);
    LCD_WRITE_DATA(0xE4);
    LCD_WRITE_DATA(0x88);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x40);

    LCD_WRITE_COMMAND(0xEC);
    LCD_WRITE_DATA(0x3C);
    LCD_WRITE_DATA(0x00);

    LCD_WRITE_COMMAND(0xED);
    LCD_WRITE_DATA(0xAB);
    LCD_WRITE_DATA(0x89);
    LCD_WRITE_DATA(0x76);
    LCD_WRITE_DATA(0x54);
    LCD_WRITE_DATA(0x02);
    LCD_WRITE_DATA(0xFF);
    LCD_WRITE_DATA(0xFF);
    LCD_WRITE_DATA(0xFF);
    LCD_WRITE_DATA(0xFF);
    LCD_WRITE_DATA(0xFF);
    LCD_WRITE_DATA(0xFF);
    LCD_WRITE_DATA(0x20);
    LCD_WRITE_DATA(0x45);
    LCD_WRITE_DATA(0x67);
    LCD_WRITE_DATA(0x98);
    LCD_WRITE_DATA(0xBA);

    LCD_WRITE_COMMAND(0xFF);
    LCD_WRITE_DATA(0x77);
    LCD_WRITE_DATA(0x01);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x00);    

    LCD_WRITE_COMMAND(0x3A);
    LCD_WRITE_DATA(0x66);
  
    LCD_WRITE_COMMAND(0x36);
    LCD_WRITE_DATA(0x00);

    LCD_WRITE_COMMAND(0x21);

    LCD_WRITE_COMMAND(0x11);
    //delay_ms(120);
    
    LCD_WRITE_COMMAND(0x29);
    delay_ms(20);

    LOG_D("=====================LCD_panel_init finish\n");
	return;
}



