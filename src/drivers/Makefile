SRC +=	$(SRC_DIR)/drivers/hid_app.c
SRC +=	$(SRC_DIR)/drivers/axp228.c

ifeq ("$(PLATFORM)", "sipeed")
SRC +=	$(SRC_DIR)/drivers/st7701s_rgb.c
endif

ifeq ("$(PLATFORM)", "devterm")
SRC +=	$(SRC_DIR)/drivers/icn9707_480x1280.c
endif

INC +=	$(SRC_DIR)/drivers