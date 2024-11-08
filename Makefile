# Variables
TOOLCHAIN_INSTALL_DIR ?= $(shell pwd)/toolchain
TARGET_NAME = app
BUILD_DIR = build
SRC_DIRS = src
INC_DIRS = src

SRC_ADD = 	src/lib/tinyusb-ohci/src/tusb.c \
			src/lib/tinyusb-ohci/src/class/cdc/cdc_host.c \
			src/lib/tinyusb-ohci/src/class/hid/hid_host.c \
			src/lib/tinyusb-ohci/src/class/msc/msc_host.c \
			src/lib/tinyusb-ohci/src/host/hub.c \
			src/lib/tinyusb-ohci/src/host/usbh.c \
			src/lib/tinyusb-ohci/src/portable/ohci/ohci.c \
			src/lib/tinyusb-ohci/examples/host/cdc_msc_hid/src/hid_app.c \
			src/lib/tinyusb-ohci/src/common/tusb_fifo.c

INC_ADD = 	src/lib/tinyusb-ohci/src

SRC_EXL = 	src/lib/tinyusb-ohci/% \
			src/aw_lib/de.c \
			src/aw_lib/dmac.c \
			src/aw_lib/timer.c \
			src/aw_lib/smhc.c \
			src/aw_lib/twi.c \
			src/aw_lib/tcon_lcd-lvds.c \
			src/aw_lib/tcon_lcd-rgb.c \
			src/aw_lib/tcon_lcd.c

INC_EXL = 	src/lib/tinyusb-ohci%

#Toolcahin
T_HEAD_DEBUGSERVER_BIN = $(TOOLCHAIN_INSTALL_DIR)/T-HEAD_DebugServer/DebugServerConsole.elf
RISCV64_GLIBC_GCC_BIN  = $(TOOLCHAIN_INSTALL_DIR)/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu-
RISCV64_MUSL_BIN = $(TOOLCHAIN_INSTALL_DIR)/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin/riscv64-unknown-linux-musl-

CROSS_COMPILE = $(RISCV64_GLIBC_GCC_BIN)
AS = ${CROSS_COMPILE}gcc
CC = ${CROSS_COMPILE}gcc
LD = ${CROSS_COMPILE}gcc
GDB = ${CROSS_COMPILE}gdb
OBJCOPY = ${CROSS_COMPILE}objcopy
OBDUMP = ${CROSS_COMPILE}objdump

DEVICE = -mcmodel=medany -march=rv64imafdc -mabi=lp64
CFLAGS = $(DEVICE) -Wno-cpp -fvar-tracking -ffreestanding -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -D_POSIX_SOURCE -fdiagnostics-color=always
AFLAGS = -c $(DEVICE) -x assembler-with-cpp -D__ASSEMBLY__
LFLAGS = $(DEVICE) -nostartfiles -Wl,--gc-sections,-Map=$(BUILD_DIR)/$(TARGET_NAME).map,-cref,-u,_start -T $(SRC_DIRS)/aw_f133_app.ld -lsupc++ -lgcc -static #--specs=kernel.specs 
LFLAGS_END = -Wl,--start-group -lc -lgcc -Wl,--end-group

CFLAGS +=  -O0 -ggdb
AFLAGS +=  -ggdb

# Includes
INCS = $(shell find $(INC_DIRS) -type d)
INCS := $(filter-out $(INC_EXL), $(INCS))
INCS += $(INC_ADD)

INC_FLAGS = $(addprefix -I,$(INCS))
INC_FLAGS += -MMD -MP

# Sources
SRCS = $(shell find $(SRC_DIRS) -name '*.c' -or -name '*.s' -or -name '*.S')
SRCS := $(filter-out $(SRC_EXL), $(SRCS))
SRCS += $(SRC_ADD)
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# TARGET_NAMEs

# Check if verbosity is ON for build process
CMD_PREFIX_DEFAULT := @
ifeq ($(v), 1)
	CMD_PREFIX :=
else
	CMD_PREFIX := $(CMD_PREFIX_DEFAULT)
endif

$(BUILD_DIR)/%.c.o: %.c
	@echo CC
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(CC) -o $@ -c $(INC_FLAGS) $(CFLAGS) $< 

$(BUILD_DIR)/%.s.o: %.s
	@echo AS
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(AS) $(AFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.S.o: %.S
	@echo AS
	$(CMD_PREFIX)mkdir -p $(dir $@)
	$(CMD_PREFIX)$(AS) $(INC_FLAGS) $(AFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(TARGET_NAME): $(OBJS)
	@echo LD
	$(CMD_PREFIX)${LD} -o $(BUILD_DIR)/$(TARGET_NAME).elf ${LFLAGS} $(OBJS) $(LFLAGS_END)
	$(CMD_PREFIX)${OBJCOPY} -O binary -S $(BUILD_DIR)/$(TARGET_NAME).elf $(BUILD_DIR)/$(TARGET_NAME).bin

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET_NAME)

dis: $(BUILD_DIR)/$(TARGET_NAME).bin
	@echo DIS
	$(CMD_PREFIX)${OBDUMP} -D -S $(BUILD_DIR)/$(TARGET_NAME).elf > $(BUILD_DIR)/$(TARGET_NAME).asm

clean:
	@echo RM
	$(CMD_PREFIX)rm -rf $(BUILD_DIR)/*

debug:
	@echo DEBUG
	@echo "${RED}Press and hold the FEL pin then press RESET pin to go to the FEL mode.${NC}"
	xfel ddr d1
	xfel jtag
	$(T_HEAD_DEBUGSERVER_BIN)&
	$(GDB) -x $(SRC_DIRS)/.gdbinit

-include $(DEPS)