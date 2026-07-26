#
# File: .gdbinit
# Author: ylyamin
#
#Set gdb environment
set confirm off
set height  0
target remote localhost:1025

#Memory layout
set $spl_addr =  0x00020000
set $app_addr =  0x40000000

#Load app
restore src/bootloader/boot0_sdcard.bin binary $spl_addr
restore build/allwinner_d1_hal.bin binary $app_addr
file src/bootloader/boot0_sdcard.elf
file build/allwinner_d1_hal.elf

echo Loading finished\n
echo Jump to address\n

hbreak _BOOT_ERROR:

hbreak usb_task_init
j *$spl_addr