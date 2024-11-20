#ifndef  __ICN9707_480X1280_H__
#define  __ICN9707_480X1280_H__
#include <tcon_lcd.h>

timing_t LCD_get_param(void);
void LCD_gpio_init(void);
void LCD_bl_open(void);
void LCD_bl_close(void);
void LCD_panel_init(void);

#endif
