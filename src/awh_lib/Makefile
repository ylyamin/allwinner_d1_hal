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

ifeq ("$(PLATFORM)", "devterm")
#SRC +=	$(SRC_DIR)/awh_lib/tcon_lcd_rgb.c
SRC +=	$(SRC_DIR)/awh_lib/tcon_lcd_lvds.c
#SRC +=	$(SRC_DIR)/awh_lib/tcon_lcd_dsi.c
#SRC +=	$(SRC_DIR)/awh_lib/mipi_dsi.c
endif

ifeq ("$(PLATFORM)", "sipeed")
#SRC +=	$(SRC_DIR)/awh_lib/tcon_lcd_rgb.c
#SRC +=	$(SRC_DIR)/awh_lib/tcon_lcd_lvds.c
endif

#SRC +=	$(SRC_DIR)/awh_lib/timer.c
SRC +=	$(SRC_DIR)/awh_lib/twi.c
SRC +=	$(SRC_DIR)/awh_lib/uart.c
SRC +=	$(SRC_DIR)/awh_lib/usb.c


INC +=	$(SRC_DIR)/awh_lib