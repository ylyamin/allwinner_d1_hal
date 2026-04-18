/*
 * Allwinner SoCs display driver.
 *
 * Copyright (C) 2021 Sipeed.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __RGB_TXW686013B0_NV3052C_H__
#define  __RGB_TXW686013B0_NV3052C_H__
#include <tcon_lcd.h>

timing_t LCD_get_param(void);
void LCD_gpio_init(void);
void LCD_bl_open(void);
void LCD_bl_close(void);
void LCD_panel_init(void);

#endif /* __RGB_TXW686013B0_NV3052C_H__ */
