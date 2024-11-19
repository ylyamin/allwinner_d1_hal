#ifndef  __ICN9707_480X1280_H__
#define  __ICN9707_480X1280_H__

void LCD_panel_init(void);


/* 
lcd_x = 480
lcd_y = 1280 


u32 g_irq_no[] = {
    106,//tcon-lcd0
    107,//tcon-tv
    108 //dsi
};

u32 g_reg_base[] = {
    0x05000000,//de0
    0x05460000,//disp_if_top
    0x05461000,//tcon_lcd0
    0x05470000,//tcon_tv
    0x05450000,//dsi0
};

        CLK_DE0,
        CLK_PLL_PERIPH0_2X,

        CLK_TCON_LCD0,
        CLK_PLL_VIDEO0_4X,

        CLK_MIPI_DSI,
        CLK_PLL_PERIPH0,

*/

#endif
