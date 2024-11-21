#
# File: .gdbinit
# Author: ylyamin
#
#Set gdb environment
set confirm off
set height  0
target remote localhost:1025

#Memory layout
set $app_addr =  0x40000000

#Load app
restore build/allwinner_d1_hal.bin binary $app_addr
file build/allwinner_d1_hal.elf

echo Loading finished\n
echo Jump to address\n
#b irq_enable
j *$app_addr