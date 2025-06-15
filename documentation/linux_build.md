# Linux build

according article https://andreas.welcomes-you.com/boot-sw-debian-risc-v-lichee-rv-2

## packages
sudo apt install git
sudo apt install make
sudo apt install build-essential
sudo apt install libfontconfig1
sudo apt install ncurses-devel
sudo apt install libncurses-dev
sudo apt install build-essential
sudo apt install flex bison
sudo apt install python3-setuptools
sudo apt install swig
sudo apt install python3-dev
sudo apt install openssl
sudo apt install libssl-dev

## toolchain
wget https://github.com/YuzukiHD/sunxi-bsp-toolchains/releases/download/1.0.0/riscv64-glibc-gcc-thead_20200702.tar.xz
tar -xf riscv64-glibc-gcc-thead_20200702.tar.xz
$HOME/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu-

## SPL
git clone https://github.com/smaeul/sun20i_d1_spl
cd sun20i_d1_spl/
git checkout 4da9c518c124d6f6123bf274e449514863df3646
make CROSS_COMPILE=$HOME/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu- p=sun20iw1p1 mmc

## opensbi
git clone https://github.com/smaeul/opensbi
cd opensbi
git checkout d1-wip
make CROSS_COMPILE=$HOME/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu- PLATFORM=generic FW_PIC=y FW_OPTIONS=0x2

## uboot
git clone https://github.com/smaeul/u-boot
cd u-boot/
git checkout d1-2022-04-05
make CROSS_COMPILE=$HOME/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu- lichee_rv_defconfig
make -j `nproc` ARCH=riscv CROSS_COMPILE=$HOME/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu- all

## TPL image
u-boot/tools/mkimage -T sunxi_toc1 -d licheerv_toc1.cfg u-boot.toc1
## boot.scr
u-boot/tools/mkimage -T script -O linux -d licheerv_u-boot-bootscr.txt boot.scr

## linux kernel
#wget https://andreas.welcomes-you.com/media/files/licheerv_linux_defconfig
wget https://andreas.welcomes-you.com/media/files/licheerv-debian-rootfs_2022-03-11.tar.xz
#wget https://andreas.welcomes-you.com/media/files/licheerv-bootsw-linux-kernel_2022-03-07.tgz
#tar -xvzf licheerv-bootsw-linux-kernel_2022-03-07.tgz

mkdir -p linux-build/arch/riscv/configs
#cp licheerv_linux_defconfig linux-build/arch/riscv/configs/licheerv_defconfig

git clone https://github.com/smaeul/linux sunxi-linux
cd sunxi-linux
git checkout d1/wip
git reset --hard origin/d1/wip
cd ..

make ARCH=riscv -C sunxi-linux/ O=$HOME/linux-build nezha_defconfig
make -j `nproc` -C linux-build ARCH=riscv CROSS_COMPILE=$HOME/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu- Image.gz dtbs

./linux_image_create.sh
./linux_image_update_kernel.sh

The login for Linux via ssh is rv/licheerv and via serial interface root/rootpwd.


## attends

d1-wip-v5.17-rc2 - work
d1-wip-v5.18-rc4 - Waiting for root device /dev/mmcblk0p2...
d1/wip - Waiting for root device /dev/mmcblk0p2...

6.10, 6.8, 6.3, 6.2 - no work
