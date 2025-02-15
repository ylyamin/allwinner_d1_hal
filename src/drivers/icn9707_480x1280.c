#include "icn9707_480x1280.h"
#include <gpio.h>
#include <ccu.h>
#include <log.h>
#include <axp228.h>
#include <tcon_lcd.h>
#include <dsi.h>

timing_t timing = {
	.lcd_type = DSI,
	.pixclk = 55000000, //907752
	.lcd_w = 480,
	.lcd_h = 1280,
	.hbp = 150, //hsync back porch(pixel) + hsync plus width(pixel)
	.ht = 694,  //hsync total cycle(pixel)
	.hspw = 40, //hsync plus width(pixel)
	.vbp = 12,  //vsync back porch(line) + vysnc plus width(line)
	.vt = 1308, //vysnc total cycle(line)
	.vspw = 10, //vysnc plus width(pixel)
};

struct gpio_t lcd_gpio[] = {
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
		.pin =  BV(20), //BL
		.pupd = GPIO_PUPD_UP,
		.mode = GPIO_MODE_OUTPUT,
		.drv =  GPIO_DRV_3,
	},
	{
		.gpio = GPIOD,
		.pin =  0x3ff, // D0-D9
		.mode = GPIO_MODE_FNC4,
		.pupd = GPIO_PUPD_OFF,
		.drv =  GPIO_DRV_3, 
		.state = GPIO_SET,
	},
};

#define panel_rst_1 gpio_set(&lcd_gpio[0], GPIO_SET)
#define panel_rst_0 gpio_set(&lcd_gpio[0], GPIO_RESET)
#define panel_bl_1  gpio_set(&lcd_gpio[1], GPIO_SET)
#define panel_bl_0  gpio_set(&lcd_gpio[1], GPIO_RESET)

#define REGFLAG_END_OF_TABLE     0x102
#define REGFLAG_DELAY            0x101

struct lcd_setting_table {
    uint16_t cmd;
    uint32_t count;
    uint8_t para_list[64];
};

static struct lcd_setting_table lcd_init_setting[] = {

	{0xF0, 2, {0x5A, 0x59} },
	{0xF1, 2, {0xA5, 0xA6} },
	{0xB0, 14, {0x54,0x32,0x23,0x45,0x44,0x44,0x44,0x44,0x9F,0x00,0x01,0x9F,0x00,0x01} },
	{0xB1, 10, {0x32,0x84,0x02,0x83,0x29,0x06,0x06,0x72,0x06,0x06} },
	{0xB2, 1, {0x73} },
	{0xB3, 20, {0x0B,0x09,0x13,0x11,0x0F,0x0D,0x00,0x00,0x00,0x03,0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x05,0x07} },
	{0xB4, 20, {0x0A,0x08,0x12,0x10,0x0E,0x0C,0x00,0x00,0x00,0x03,0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x04,0x06} },
	{0xB6, 2, {0x13,0x13} },
	{0xB8, 4, {0xB4,0x43,0x02,0xCC} },
	{0xB9, 4, {0xA5,0x20,0xFF,0xC8} },
	{0xBA, 2, {0x88,0x23} },
	{0xBD, 10, {0x43,0x0E,0x0E,0x50,0x50,0x29,0x10,0x03,0x44,0x03} },
	{0xC1, 8, {0x00,0x0C,0x16,0x04,0x00,0x30,0x10,0x04} },
	{0xC2, 2, {0x21,0x81} },
	{0xC3, 2, {0x02,0x30} },
	{0xC7, 2, {0x25,0x6A} },
	{0xC8, 38, {0x7C,0x68,0x59,0x4E,0x4B,0x3C,0x41,0x2B,0x44,0x43,0x43,0x60,0x4E,0x55,0x47,0x44,0x38,0x27,0x06,0x7C,0x68,0x59,0x4E,0x4B,0x3C,0x41,0x2B,0x44,0x43,0x43,0x60,0x4E,0x55,0x47,0x44,0x38,0x27,0x06} },
	{0xD4, 6, {0x00,0x00,0x00,0x32,0x04,0x51} },
	{0xF1, 2, {0x5A,0x59} },
	{0xF0, 2, {0xA5,0xA6} },
	{0x36, 1, {0x14} },
	{0x35, 1, {0x00} },
	
	{0x11, 1, {0x00} },
	{REGFLAG_DELAY, 120, {} },
	{0x29, 1, {0x00} },	
	{REGFLAG_DELAY, 20, {} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }

};

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
	
    uint32_t i;
    LOG_D("<0>raoyiming +++ LCD_panel_init\n");

    /*all off*/
    axp_LCD_control(TWI0,0);
    delay_ms(100);

    /*start*/
    panel_rst_1;
    axp_LCD_control(TWI0,1);

    /*T2*/
    delay_ms(10);
    panel_rst_0;

    /*T3*/
    delay_ms(20);

	dsi_init();


    /*init sequence*/
/*     for (i = 0; ; i++) {
        if(lcd_init_setting[i].cmd == REGFLAG_END_OF_TABLE) {
            break;
        } else if (lcd_init_setting[i].cmd == REGFLAG_DELAY) {
            delay_ms(lcd_init_setting[i].count);
        } else {
            //!dsi_dcs_wr(sel, (uint8_t)lcd_init_setting[i].cmd, lcd_init_setting[i].para_list, lcd_init_setting[i].count);
        }
    } */

    //!sunxi_lcd_dsi_clk_enable(sel);
	/* T6 */
	delay_ms(120);
    LOG_D("<0>raoyiming +++ LCD_panel_init finish\n");
}