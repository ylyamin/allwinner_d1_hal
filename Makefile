# Variables
TARGET = app
BUILD_DIR = build

SRC_DIRS = src
SRC_ADD = 
SRC_EXL = 	src/aw_lib/de.c \
			src/aw_lib/dmac.c \
			src/aw_lib/timer.c \
			src/aw_lib/smhc.c \
			src/aw_lib/twi.c \
			src/aw_lib/tcon_lcd-lvds.c \
			src/aw_lib/tcon_lcd-rgb.c \
			src/aw_lib/tcon_lcd.c

#Toolcahin
TOOLCHAIN_INSTALL_DIR ?= $(shell pwd)/toolchain
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
LFLAGS = $(DEVICE) -nostartfiles -Wl,--gc-sections,-Map=$(BUILD_DIR)/$(TARGET).map,-cref,-u,_start -T $(SRC_DIRS)/aw_f133_app.ld -lsupc++ -lgcc -static #--specs=kernel.specs 
LFLAGS_END = -Wl,--start-group -lc -lgcc -Wl,--end-group

# CFLAGS = -march=rv64gvxtheadc906 -mabi=lp64d -mcmodel=medany -fno-stack-protector --freestanding
# AFLAGS = 
# LFLAGS = --no-relax -T $(SRC_DIRS)/link.ld 
# LFLAGS_END =


CFLAGS +=  -O0 -ggdb
AFLAGS +=  -ggdb

# Sources
SRCS = $(shell find $(SRC_DIRS) -name '*.c' -or -name '*.s' -or -name '*.S')
SRCS := $(filter-out $(SRC_EXL), $(SRCS))
SRCS += $(SRC_ADD)
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

INC_DIRS = $(shell find $(SRC_DIRS) -type d)
INC_FLAGS = $(addprefix -I,$(INC_DIRS))
CPPFLAGS = $(INC_FLAGS) -MMD -MP

# Targets

$(BUILD_DIR)/%.c.o: %.c
	@echo $(SRCS)
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $(CPPFLAGS) $(CFLAGS) $< 

$(BUILD_DIR)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(AS) $(CPPFLAGS) $(AFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(TARGET): $(OBJS)
	${LD} -o $(BUILD_DIR)/$(TARGET).elf ${LFLAGS} $(OBJS) $(LFLAGS_END)
	${OBJCOPY} -O binary -S $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET)

dis: $(BUILD_DIR)/$(TARGET).bin
	${OBDUMP} -D -S $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).asm

clean:
	rm -rf $(BUILD_DIR)/*

debug:
	@echo "${RED}Press and hold the FEL pin then press RESET pin to go to the FEL mode.${NC}"
	xfel ddr d1
	xfel jtag
	$(T_HEAD_DEBUGSERVER_BIN)&
	$(GDB) -x .gdbinit

-include $(DEPS)