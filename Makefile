#
# File: Makefile
# Author: ylyamin
#

# Variables
TOOLCHAIN_INSTALL_DIR ?= $(shell pwd)/toolchain
TARGET_NAME = app
BUILD_DIR = build
SRC_DIR = src
.DEFAULT_GOAL := all

# Sources
SRC +=	$(SRC_DIR)/aw_f133/memcpy_sunxi.c
SRC +=	$(SRC_DIR)/aw_f133/memset_sunxi.c

#SRC +=	$(SRC_DIR)/aw_lib/cache-c906.c
SRC +=	$(SRC_DIR)/aw_lib/ccu.c
#SRC +=	$(SRC_DIR)/aw_lib/de.c
SRC +=	$(SRC_DIR)/aw_lib/de_scaler_table.c
#SRC +=	$(SRC_DIR)/aw_lib/dmac.c
SRC +=	$(SRC_DIR)/aw_lib/gpio.c
SRC +=	$(SRC_DIR)/aw_lib/gr.c
SRC +=	$(SRC_DIR)/aw_lib/irq.c	
SRC +=	$(SRC_DIR)/aw_lib/led.c
#SRC +=	$(SRC_DIR)/aw_lib/smhc.c
#SRC +=	$(SRC_DIR)/aw_lib/tcon_lcd.c
#SRC +=	$(SRC_DIR)/aw_lib/tcon_lcd-lvds.c
#SRC +=	$(SRC_DIR)/aw_lib/tcon_lcd-rgb.c
#SRC +=	$(SRC_DIR)/aw_lib/timer.c
#SRC +=	$(SRC_DIR)/aw_lib/twi.c
SRC +=	$(SRC_DIR)/aw_lib/uart.c

SRC +=	$(SRC_DIR)/lib/tinyprintf/tinyprintf.c

SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/tusb.c
SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/typec/usbc.c
SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/class/cdc/cdc_host.c
SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/class/hid/hid_host.c
SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/class/msc/msc_host.c
SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/host/hub.c
SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/host/usbh.c
SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/common/tusb_fifo.c
SRC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src/portable/ohci/ohci.c
SRC +=	$(SRC_DIR)/hid_app.c

#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/src/tusb.c
#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/src/class/cdc/cdc_host.c
#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/src/class/hid/hid_host.c
#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/src/class/msc/msc_host.c
#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/src/host/hub.c
#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/src/host/usbh.c
#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/src/common/tusb_fifo.c
#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/examples/host/cdc_msc_hid/src/hid_app.c
#SRC +=	$(SRC_DIR)/lib/tinyusb-ohci/src/portable/ohci/ohci.c

#SRC +=	$(SRC_DIR)/kvm.c
SRC +=	$(SRC_DIR)/main.c
SRC +=	$(SRC_DIR)/start.s
SRC +=	$(SRC_DIR)/usb_task.c

INC +=	$(SRC_DIR)/aw_f133
INC +=	$(SRC_DIR)/aw_lib
INC +=	$(SRC_DIR)/lib/tinyprintf
INC +=	$(SRC_DIR)/lib/hftrx_tinyusb/src
INC +=	$(SRC_DIR)

#Toolcahin
T_HEAD_DEBUGSERVER_BIN = $(TOOLCHAIN_INSTALL_DIR)/T-HEAD_DebugServer/DebugServerConsole.elf
RISCV64_GLIBC_GCC_BIN  = $(TOOLCHAIN_INSTALL_DIR)/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu-
RISCV64_MUSL_BIN = $(TOOLCHAIN_INSTALL_DIR)/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin/riscv64-unknown-linux-musl-

XUANTIE_900_GCC_ELF_NEWLIB_DIR = $(TOOLCHAIN_INSTALL_DIR)/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1
XUANTIE_900_GCC_ELF_NEWLIB_BIN = $(XUANTIE_900_GCC_ELF_NEWLIB_DIR)/bin/riscv64-unknown-elf-

$(XUANTIE_900_GCC_ELF_NEWLIB_DIR):
	wget -P $(TOOLCHAIN_INSTALL_DIR) https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource//1705395512373/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1-20240115.tar.gz
	tar -C $(TOOLCHAIN_INSTALL_DIR) -xvzf $(TOOLCHAIN_INSTALL_DIR)/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1-20240115.tar.gz

#https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v14.2.0-2/xpack-riscv-none-elf-gcc-14.2.0-2-linux-x64.tar.gz

XPACK_RISCV_NONE_ELF_GCC_BIN = $(TOOLCHAIN_INSTALL_DIR)/xpack-riscv-none-elf-gcc-14.2.0-2/bin/riscv-none-elf-

toolchain: $(XUANTIE_900_GCC_ELF_NEWLIB_DIR)

# Compile
CROSS_COMPILE = $(XPACK_RISCV_NONE_ELF_GCC_BIN)
AS = ${CROSS_COMPILE}gcc
CC = ${CROSS_COMPILE}gcc
LD = ${CROSS_COMPILE}gcc
GDB = ${CROSS_COMPILE}gdb
OBJCOPY = ${CROSS_COMPILE}objcopy
OBDUMP = ${CROSS_COMPILE}objdump
SIZE = ${CROSS_COMPILE}size

#DEVICE = -march=rv64gcv0p7_xtheadc -mabi=lp64d -mtune=c906 -mcmodel=medlow  

DEVICE = -march=rv64imafd_zicsr -mabi=lp64d -mcmodel=medany  
CFLAGS = $(DEVICE) -fno-stack-protector -ffunction-sections -fdata-sections -fdiagnostics-color=always -Wno-cpp
AFLAGS = -c $(DEVICE) -x assembler-with-cpp
LFLAGS = $(DEVICE) -T $(SRC_DIR)/link.ld -Wl,--cref,-Map=$(BUILD_DIR)/$(TARGET_NAME).map,--print-memory-usage -nostartfiles

# -ffreestanding -std=gnu99 
# -mstrict-align
#-gc-sections 
#-lgcc
#--specs=nano.specs -fno-common -fno-builtin
# -nostartfiles 
# -nostdlib

CFLAGS +=  -O0 -ggdb
AFLAGS +=  -ggdb

# Sources
OBJS = $(SRC:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# Includes
INC_FLAGS = $(addprefix -I,$(INC))
INC_FLAGS += -MMD -MP

# Targets

# Check if verbosity is ON (v=1) for build process
CMD_PREFIX_DEFAULT := @
ifeq ($(v), 1)
	CMD_PREFIX :=
else
	CMD_PREFIX := $(CMD_PREFIX_DEFAULT)
endif

$(BUILD_DIR)/%.c.o: %.c
	@echo CC $<
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(CC) -o $@ -c $(INC_FLAGS) $(CFLAGS) $< 

$(BUILD_DIR)/%.s.o: %.s
	@echo AS $<
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(AS) $(INC_FLAGS) $(AFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.S.o: %.S
	@echo AS $<
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(AS) $(INC_FLAGS) $(AFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(TARGET_NAME).bin: $(OBJS)
	@echo LD $(BUILD_DIR)/$(TARGET_NAME).elf
	$(CMD_PREFIX)${LD} -o $(BUILD_DIR)/$(TARGET_NAME).elf ${LFLAGS} $(OBJS)
	$(CMD_PREFIX)${OBJCOPY} -O binary -S $(BUILD_DIR)/$(TARGET_NAME).elf $(BUILD_DIR)/$(TARGET_NAME).bin

all: $(BUILD_DIR)/$(TARGET_NAME).bin

dis: $(BUILD_DIR)/$(TARGET_NAME).bin
	@echo DISASSEMBLY $(BUILD_DIR)/$(TARGET_NAME).asm
	$(CMD_PREFIX)${OBDUMP} -D -S $(BUILD_DIR)/$(TARGET_NAME).elf > $(BUILD_DIR)/$(TARGET_NAME).asm

clean:
	@echo REMOVE $(BUILD_DIR)/
	$(CMD_PREFIX)rm -rf $(BUILD_DIR)/*

flash:
	@echo FLASH $(BUILD_DIR)/$(TARGET_NAME).bin
	$(CMD_PREFIX)xfel ddr d1
	$(CMD_PREFIX)xfel write 0x40000000 $(BUILD_DIR)/$(TARGET_NAME).bin
	$(CMD_PREFIX)xfel exec 0x40000000

debug:
	@echo DEBUG $(SRC_DIR)/.gdbinit
	xfel ddr d1
	xfel jtag
	$(T_HEAD_DEBUGSERVER_BIN)&
	$(GDB) -x $(SRC_DIR)/.gdbinit

-include $(DEPS)

.PHONY: all clean