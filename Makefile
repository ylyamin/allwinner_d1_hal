#
# File: Makefile
# Author: ylyamin
#

# Variables
VERSION_GIT=$(shell if [ -d .git ]; then git describe --tags 2> /dev/null; fi)
TOOLCHAIN_INSTALL_DIR ?= $(shell pwd)/toolchain
DEBUGGER_INSTALL_DIR ?= $(shell pwd)/debugger
TARGET_NAME = allwinner_d1_hal
BUILD_DIR = build
SRC_DIR = src

SOC = d1h
PLATFORM = sipeed
SD_IMAGE = image/$(PLATFORM)_sd_image.img
SD_MOUNT = /dev/sdb
BOOTLOADER_NAME = boot0_sdcard_sun20iw1p1_d1h.bin

.DEFAULT_GOAL := all

# SoC: d1h or d1s

SOC_D = h
ifeq ("$(origin soc)", "command line")
	SOC = $(soc)
	ifeq ("$(SOC)","d1h")
		SOC_D = h
		BOOTLOADER_NAME = boot0_sdcard_sun20iw1p1_d1h.bin
	endif

	ifeq ("$(SOC)","d1s")
		SOC_D = s
		BOOTLOADER_NAME = boot0_sdcard_sun20iw1p1_d1s.bin
	endif
endif

# Platform: devterm or sipeed

PLATFORM_D = s
ifeq ("$(origin platform)", "command line")
	PLATFORM = $(platform)
	ifeq ("$(PLATFORM)","sipeed")
		PLATFORM_D = s
	endif

	ifeq ("$(PLATFORM)","devterm")
		PLATFORM_D = d
	endif
endif

# Check if verbosity is ON (v=1) for build process
CMD_PREFIX_DEFAULT := @
ifeq ($(v), 1)
	CMD_PREFIX :=
else
	CMD_PREFIX := $(CMD_PREFIX_DEFAULT)
endif

# Sources
SRC +=	$(SRC_DIR)/board_start.c
SRC +=	$(SRC_DIR)/board_start.s
SRC +=	$(SRC_DIR)/main.c

INC +=	$(SRC_DIR)

include $(SRC_DIR)/components/Makefile
include $(SRC_DIR)/drivers/Makefile
include $(SRC_DIR)/aw_f133/Makefile
include $(SRC_DIR)/aw_lib/Makefile
include $(SRC_DIR)/lib/Makefile

#Toolcahin
XFEL_DIR = $(TOOLCHAIN_INSTALL_DIR)/xfel
T_HEAD_DEBUGSERVER_DIR = $(TOOLCHAIN_INSTALL_DIR)/T-HEAD_DebugServer
RISCV64_MUSL_DIR = $(TOOLCHAIN_INSTALL_DIR)/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu
RISCV64_GLIBC_GCC_DIR = $(TOOLCHAIN_INSTALL_DIR)/riscv64-glibc-gcc-thead_20200702
XUANTIE_900_GCC_ELF_NEWLIB_DIR = $(TOOLCHAIN_INSTALL_DIR)/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1
XPACK_RISCV_NONE_ELF_GCC_DIR = $(TOOLCHAIN_INSTALL_DIR)/xpack-riscv-none-elf-gcc-14.2.0-2

T_HEAD_DEBUGSERVER_BIN = $(T_HEAD_DEBUGSERVER_DIR)/DebugServerConsole.elf
RISCV64_MUSL_BIN = $(RISCV64_MUSL_DIR)/bin/riscv64-unknown-linux-musl-
RISCV64_GLIBC_GCC_BIN = $(RISCV64_GLIBC_GCC_DIR)/bin/riscv64-unknown-linux-gnu-
XUANTIE_900_GCC_ELF_NEWLIB_BIN = $(XUANTIE_900_GCC_ELF_NEWLIB_DIR)/bin/riscv64-unknown-elf-
XPACK_RISCV_NONE_ELF_GCC_BIN = $(XPACK_RISCV_NONE_ELF_GCC_DIR)/bin/riscv-none-elf-

$(XFEL_DIR):
	@echo XFEL INSTALL
	$(CMD_PREFIX)git clone https://github.com/xboot/xfel $(XFEL_DIR)
	$(CMD_PREFIX)cd $(XFEL_DIR) && make && sudo make install

$(XFEL_DIR)-remove:
	@echo XFEL REMOVE
	$(CMD_PREFIX)rm -rf $(XFEL_DIR)

$(T_HEAD_DEBUGSERVER_DIR):
	@echo T_HEAD_DEBUGSERVER INSTALL
	$(CMD_PREFIX)wget -P $(TOOLCHAIN_INSTALL_DIR) https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource//1666331533949/T-Head-DebugServer-linux-x86_64-V5.16.5-20221021.sh.tar.gz
	$(CMD_PREFIX)tar -C $(TOOLCHAIN_INSTALL_DIR) -xvzf $(TOOLCHAIN_INSTALL_DIR)/T-Head-DebugServer-linux-x86_64-V5.16.5-20221021.sh.tar.gz 
	$(CMD_PREFIX)cd $(TOOLCHAIN_INSTALL_DIR) && sudo $(TOOLCHAIN_INSTALL_DIR)/T-Head-DebugServer-linux-x86_64-V5.16.5-20221021.sh -i

$(T_HEAD_DEBUGSERVER_DIR)-remove:
	@echo T_HEAD_DEBUGSERVER REMOVE
	$(CMD_PREFIX)rm -rf $(TOOLCHAIN_INSTALL_DIR)/T-Head-DebugServer*
	$(CMD_PREFIX)sudo rm -rf $(TOOLCHAIN_INSTALL_DIR)/T-HEAD_DebugServer*

$(XUANTIE_900_GCC_ELF_NEWLIB_DIR):
	@echo XUANTIE_900_GCC_ELF_NEWLIB INSTALL
	$(CMD_PREFIX)wget -P $(TOOLCHAIN_INSTALL_DIR) https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource//1705395512373/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1-20240115.tar.gz
	$(CMD_PREFIX)tar -C $(TOOLCHAIN_INSTALL_DIR) -xvzf $(TOOLCHAIN_INSTALL_DIR)/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1-20240115.tar.gz

$(XUANTIE_900_GCC_ELF_NEWLIB_DIR)-remove:
	@echo XUANTIE_900_GCC_ELF_NEWLIB REMOVE
	$(CMD_PREFIX)rm -rf $(TOOLCHAIN_INSTALL_DIR)/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1*

$(XPACK_RISCV_NONE_ELF_GCC_DIR):
	@echo XPACK_RISCV_NONE_ELF_GCC INSTALL
	$(CMD_PREFIX)wget -P $(TOOLCHAIN_INSTALL_DIR) https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v14.2.0-2/xpack-riscv-none-elf-gcc-14.2.0-2-linux-x64.tar.gz
	$(CMD_PREFIX)tar -C $(TOOLCHAIN_INSTALL_DIR) -xvzf $(TOOLCHAIN_INSTALL_DIR)/xpack-riscv-none-elf-gcc-14.2.0-2-linux-x64.tar.gz

$(XPACK_RISCV_NONE_ELF_GCC_DIR)-remove:
	@echo XPACK_RISCV_NONE_ELF_GCC REMOVE
	$(CMD_PREFIX)rm -rf $(TOOLCHAIN_INSTALL_DIR)/xpack-riscv-none-elf-gcc-14.2.0-2

toolchain: $(XFEL_DIR) $(T_HEAD_DEBUGSERVER_DIR) $(XPACK_RISCV_NONE_ELF_GCC_DIR)

toolchain-remove: $(XFEL_DIR)-remove $(T_HEAD_DEBUGSERVER_DIR)-remove $(XPACK_RISCV_NONE_ELF_GCC_DIR)-remove

#Debugger
$(DEBUGGER_INSTALL_DIR)/blisp:
	$(CMD_PREFIX)wget -P $(DEBUGGER_INSTALL_DIR) https://github.com/bouffalolab/bouffalo_sdk/blob/master/tools/cklink_firmware/bl702_cklink_whole_img_v2.2.bin
	$(CMD_PREFIX)wget -P $(DEBUGGER_INSTALL_DIR) https://github.com/pine64/blisp/releases/download/v0.0.4/blisp-linux-x86_64-v0.0.4.zip
	$(CMD_PREFIX)unzip $(DEBUGGER_INSTALL_DIR)/blisp-linux-x86_64-v0.0.4.zip -d $(DEBUGGER_INSTALL_DIR)

debug-burn: $(DEBUGGER_INSTALL_DIR)/blisp
	@echo DEBUGGER BURN
	@echo "Press and hold the boot pin then plug the usb in the computer to go to the boot mode."
	$(CMD_PREFIX)$(DEBUGGER_INSTALL_DIR)/blisp iot -c bl70x --reset -s $(DEBUGGER_INSTALL_DIR)/bl702_cklink_whole_img_v2.2.bin -l 0x0

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
VARS   = -D VERSION_GIT="\"$(VERSION_GIT)\"" -D SOC="'$(SOC_D)'" -D PLATFORM="'$(PLATFORM_D)'"
CFLAGS = $(DEVICE) $(VARS) -fno-stack-protector -ffunction-sections -fdata-sections -fdiagnostics-color=always -Wno-cpp -Wno-int-conversion
AFLAGS = $(DEVICE) $(VARS) -x assembler-with-cpp
LFLAGS = $(DEVICE) $(VARS) -T $(SRC_DIR)/link.ld -Wl,--cref,-Map=$(BUILD_DIR)/$(TARGET_NAME).map,--print-memory-usage -nostartfiles 

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

$(BUILD_DIR)/%.c.o: %.c
	@echo CC $<
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(CC) $(INC_FLAGS) $(CFLAGS) -c $< -o $@ 

$(BUILD_DIR)/%.s.o: %.s
	@echo AS $<
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(AS) $(INC_FLAGS) $(AFLAGS) -c  $< -o $@

$(BUILD_DIR)/%.S.o: %.S
	@echo AS $<
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(AS) $(INC_FLAGS) $(AFLAGS) -c  $< -o $@

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

submodules:
	@echo SUBMODULES
	git submodule update --init

#SD card
$(SD_IMAGE):
	@echo SD $(SD_IMAGE)
	$(CMD_PREFIX)$(SRC_DIR)/bootloader/mkimage -T sunxi_toc1 -d $(SRC_DIR)/bootloader/toc1_D1H.cfg $(BUILD_DIR)/$(TARGET_NAME)_sd.bin
	$(CMD_PREFIX)dd if=$(SRC_DIR)/bootloader/$(BOOTLOADER_NAME) of=$(SD_IMAGE) bs=8192 seek=16
	$(CMD_PREFIX)dd if=$(BUILD_DIR)/$(TARGET_NAME)_sd.bin of=$(SD_IMAGE) bs=512 seek=32800

sd: $(SD_IMAGE)

sd-burn:
	@echo SD BURN
	$(CMD_PREFIX)dd if=$(SD_IMAGE) of=$(SD_MOUNT)

sd-clean:
	@echo REMOVE $(SD_IMAGE)
	$(CMD_PREFIX)rm -rf $(SD_IMAGE)

-include $(DEPS)

.PHONY: all clean