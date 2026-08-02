ifeq ($(PLATFORM_D),d)
SRC +=	$(SRC_DIR)/drivers/axp228.c
endif

ifeq ($(LCD_D),l)
SRC +=	$(SRC_DIR)/drivers/LVDS_TXW686017B0_NV3051F.c

else ifeq ($(LCD_D),m)
SRC +=	$(SRC_DIR)/drivers/icn9707_480x1280.c

else ifeq ($(LCD_D),r)
SRC +=	$(SRC_DIR)/drivers/RGB_043026_N6_ST7701S.c
#SRC +=	$(SRC_DIR)/drivers/RGB_TXW686013B0_NV3052C.c

endif

INC +=	$(SRC_DIR)/drivers