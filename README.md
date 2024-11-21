# Overview

Elementary HAL (Hardware Abstraction Layer) for Allwinner D1 SOC.

# Features
- Working graphics with LCD RGB, LCD MIPI not yet
- USB Host with support USB 2.0/1.1 Keyboard, Mouse
- TWI (I2C)
- UART, GPIO

# Executed at hardware such as:
- [ClockworkPi DevTerm R-01](https://www.clockworkpi.com/home-devterm)
- [Sipeed Lichee RV + Dock](https://wiki.sipeed.com/hardware/en/lichee/RV/Dock.html)

# Build

```sh
make
```

```sh
make flash
```

# Reused:

aw_f133 aw_f133_app.ld crt_RV64IFDV.S from https://github.com/ua1arn/hftrx/tree/develop-linux/arch/aw_f133
aw_lib from https://github.com/robots/allwinner_t113/tree/master/common/aw
exeptions from https://github.com/xboot/xboot
memcpy.s memset.S from https://github.com/xboot/xfel/tree/master/payloads/d1_f133/d1-ddr/source


# TODO:
- LCD mipi

- Uart for D1s (configured gpio)
- Cleanup repo structure, readme etc.
- Rename repo
- Version
- Bootloader external

- Do as library and external main function
- ehci/ohci auto switch
- MMU
- No libc ?
- simple linker script ?
- FreeRTOS ?
- Own bootloader
- Changelog
- Kbuild ?
- Common for D1s / T113


