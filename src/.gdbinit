#Set gdb environment
set confirm off
set height  0
target remote localhost:1025

#Memory layout
set $app_addr =  0x40000000

#Load app
restore build/app.bin binary $app_addr
file build/app.elf

echo Loading finished\n
echo Jump to address\n
#b main
j *$app_addr