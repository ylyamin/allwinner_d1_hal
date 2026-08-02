#SRC +=	$(SRC_DIR)/awh_lib/cache-c906.c
SRC +=	$(SRC_DIR)/awh_lib/ccu.c
SRC +=	$(SRC_DIR)/awh_lib/de.c
SRC +=	$(SRC_DIR)/awh_lib/de_scaler_table.c
#SRC +=	$(SRC_DIR)/awh_lib/dmac.c
SRC +=	$(SRC_DIR)/awh_lib/gpio.c
SRC +=	$(SRC_DIR)/awh_lib/gr.c
SRC +=	$(SRC_DIR)/awh_lib/irq.c	
SRC +=	$(SRC_DIR)/awh_lib/led.c
#SRC +=	$(SRC_DIR)/awh_lib/smhc.c
SRC +=	$(SRC_DIR)/awh_lib/fifo.c
SRC +=	$(SRC_DIR)/awh_lib/g2d.c
SRC +=	$(SRC_DIR)/awh_lib/log.c
SRC +=	$(SRC_DIR)/awh_lib/thermal.c
#SRC +=	$(SRC_DIR)/awh_lib/timer.c

ifeq ($(LCD_D),l)
SRC +=	$(SRC_DIR)/awh_lib/tcon_lcd_lvds.c

else ifeq ($(LCD_D),m)
SRC +=	$(SRC_DIR)/awh_lib/mipi_dsi.c
SRC +=	$(SRC_DIR)/awh_lib/tcon_lcd_dsi.c

else ifeq ($(LCD_D),r)
SRC +=	$(SRC_DIR)/awh_lib/tcon_lcd_rgb.c

endif

SRC +=	$(SRC_DIR)/awh_lib/twi.c
SRC +=	$(SRC_DIR)/awh_lib/uart.c
SRC +=	$(SRC_DIR)/awh_lib/usb.c

INC +=	$(SRC_DIR)/awh_lib