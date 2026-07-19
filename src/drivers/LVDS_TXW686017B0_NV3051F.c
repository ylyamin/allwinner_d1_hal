#include <LVDS_TXW686017B0_NV3051F.h>
#include <gpio.h>
#include <ccu.h>
#include <log.h>
#include <axp228.h>
/*
LVDS panel TXW686017B0 with IC NV3051F.
Shenzhen Tian xian wei Technology.

//#define HDP   600//玻璃实际(480) 需两边插黑处理 Horizontal 
//#define VDP   1280    //vertiacal

//#define VSW  2 //Vertical Synchronization
//#define VBP   8 //Vertical Back Porch
//#define VFP   16 //Vertical Front Porch

//#define HSW  2 //Horizontal Synchronization ?
//#define HBP   44 //Horizontal Back Porch
//#define HFP   46  //Horizontal Front Porch

//frame rate=60Hz////
//Pixel Clk=54.23Mhz//(480:44.82Mhz)

SPI_RES=1;
Delay(10);	//延迟10ms
SPI_RES=0;
Delay(220);//延迟220ms
SPI_RES=1;
Delay(120);  //延迟120ms          

Delay(20);  //延迟20ms  
送平台数据

1	VCOM/NC		
2	VDD 3.3V 		
3	VDD 3.3V 		
4	NC		
5	RESET 3.3       PG13
6	STBYB/NC 		
7	GND		
8	RXIN0-		    PD1
9	RXIN0+	    	PD0
10	GND		
11	RXIN1-		    PD3
12	RXIN1+		    PD2
13	GND		
14	RXIN2-		    PD5
15	RXIN2+		    PD4
16	GND		
17	RXCLKIN-		PD7
18	RXCLKIN+		PD6
19	GND		
20	RXIN3-		    PD9
21	RXIN3+		    PD8
22	GND		
23	CS		        PD10
24	SCL		        PD11
25	GND		
26	SDA		        PD12
27	SDO		        PD13
28	SELB(NC)		
29	AVDD(NC) 		
30	GND		
31	LED-  		    LEDK
32	LED-  	        	
33	L/R(NC)		
34	U/D(NC)		
35	VGL(NC)		
36	TE  		
37	NC		
38	VGH(NC)		
39	LED+ 		    LEDA
40	LED+ 	        	
*/



timing_t timing = {
    .lcd_type = LVDS,
	.pixclk = 54000000, 	//54.23Mhz
	.lcd_w = 600,      		//width 
	.lcd_h = 1280,       	//hight
    .lcd_scale_w = 480,
    .lcd_scale_h = 1280,

	.hbp = 44,          //horizontal back porch
	.ht = 600+44+46+2,  //horizontal Total Size
	.hspw = 2,          //horizontal Sync Pulse Width

	.vbp = 8,           //vertical back porch
	.vt = 1280+8+16+2,  //vertical Total Size
	.vspw = 2,          //vertical Sync Pulse Width
};

//dotclk = fframe × (X + HBP + HFP + HSPW) × (Y + VBP + VFP + VSPW) 54,225,120

struct gpio_t lcd_gpio[] = {
 	{
		.gpio = GPIOD,
		.pin = BV(19),  //RST
		.mode = GPIO_MODE_OUTPUT,
        .pupd = GPIO_PUPD_UP,
		.drv =  GPIO_DRV_3,
        .state = GPIO_RESET,
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
    
/*     {
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
	},  */
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
    LOG_D("LCD_panel_init 3051\n");

	panel_reset_1;
	delay_ms(5);  
	axp_LCD_control(TWI0,1);
	delay_ms(10);  //10
    panel_reset_0;
	delay_us(220); //220
    panel_reset_1;
	delay_ms(120); //120 

    LOG_D("LCD_panel_init finish\n");
}



