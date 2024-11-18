# TODO:

- LCD rgb/ mipi

- to do as library and external main
- cleanup repo structure, readme etc.
- ehci/ohci switch ?
- simple linker script ?
- MMU ?
- no stdlib, built-in  printf
- FreeRTOS ?
- bootloader external
- own bootloader
- changelog
- kbuild ?
- common for D1s T113 (configured gpio)

# Reuse:

aw_f133 aw_f133_app.ld crt_RV64IFDV.S from https://github.com/ua1arn/hftrx/tree/develop-linux/arch/aw_f133
tinyusb-ohci aw_lib from https://github.com/robots/allwinner_t113/tree/master/common/aw
irq-d1.c exeptions from https://github.com/xboot/xboot
memcpy.s memset.S from https://github.com/xboot/xfel/tree/master/payloads/d1_f133/d1-ddr/source


# Results:

```sh
[0:] USBH DEVICE ATTACH
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
Full Speed
[0:0] Open EP0 with Size = 8
Get 8 byte of Device Descriptor
[0:0] Get Descriptor: 80 06 00 01 00 00 08 00
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 8 bytes: OK
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 80 with 8 bytes: OK
[0:0] Control data:
  0000:  12 01 10 01 00 00 00 40                          |.......@|
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 0 bytes: OK

Set Address = 1
[0:0] Set Address: 00 05 01 00 00 00 00 00
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 8 bytes: OK
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 80 with 0 bytes: OK

[0:1] Open EP0 with Size = 64
Get Device Descriptor
[0:1] Get Descriptor: 80 06 00 01 00 00 12 00
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 8 bytes: OK
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 80 with 18 bytes: OK
[0:1] Control data:
  0000:  12 01 10 01 00 00 00 40 7B 06 03 23 00 03 01 02  |.......@{..#....|
  0010:  00 01                                            |..|
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 0 bytes: OK

Get Configuration[0] Descriptor (9 bytes)
[0:1] Get Descriptor: 80 06 00 02 00 00 09 00
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 8 bytes: OK
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 80 with 9 bytes: OK
[0:1] Control data:
  0000:  09 02 27 00 01 01 00 80 32                       |..'.....2|
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 0 bytes: OK

Get Configuration[0] Descriptor
[0:1] Get Descriptor: 80 06 00 02 00 00 27 00
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 8 bytes: OK
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 80 with 39 bytes: OK
[0:1] Control data:
  0000:  09 02 27 00 01 01 00 80 32 09 04 00 00 03 FF 00  |..'.....2.......|
  0010:  00 00 07 05 81 03 0A 00 01 07 05 02 02 40 00 00  |.............@..|
  0020:  07 05 83 02 40 00 00                             |....@..|
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 0 bytes: OK

Set Configuration = 1
[0:1] Set Configuration: 00 09 01 00 00 00 00 00
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 00 with 8 bytes: OK
[DBG]:Interrupt 11: Machine external interrupt
[DBG]:Handle irq: 50
on EP 80 with 0 bytes: OK
[1] Aborted transfer on EP 01
[1] Aborted transfer on EP 81
[1] Aborted transfer on EP 02
[1] Aborted transfer on EP 82
[1] Aborted transfer on EP 03
[1] Aborted transfer on EP 83
[1] Aborted transfer on EP 04
[1] Aborted transfer on EP 84
[1] Aborted transfer on EP 05
[1] Aborted transfer on EP 85
[1] Aborted transfer on EP 06
[1] Aborted transfer on EP 86
[1] Aborted transfer on EP 07
[1] Aborted transfer on EP 87
[1] Aborted transfer on EP 08
[1] Aborted transfer on EP 88
[1] Aborted transfer on EP 09
[1] Aborted transfer on EP 89
[1] Aborted transfer on EP 0A
[1] Aborted transfer on EP 8A
[1] Aborted transfer on EP 0B
[1] Aborted transfer on EP 8B
[1] Aborted transfer on EP 0C
[1] Aborted transfer on EP 02
[1] Aborted transfer on EP 82
[1] Aborted transfer on EP 03
[1] Aborted transfer on EP 83
[1] Aborted transfer on EP 04
[1] Aborted transfer on EP 84
[1] Aborted transfer on EP 05
[1] Aborted transfer on EP 85
[1] Aborted transfer on EP 06
[1] Aborted transfer on EP 86
[1] Aborted transfer on EP 07
[1] Aborted transfer on EP 87
[1] Aborted transfer on EP 08
[1] Aborted transfer on EP 88
[1] Aborted transfer on EP 09
[1] Aborted transfer on EP 89
[1] Aborted transfer on EP 0A
[1] Aborted transfer on EP 8A
[1] Aborted transfer on EP 0B
[1] Aborted transfer on EP 8B
[1] Aborted transfer on EP 0C
[1] Aborted transfer on EP 8C
[1] Aborted transfer on EP 0D
[1] Aborted transfer on EP 8D
[1] Aborted transfer on EP 0E
[1] Aborted transfer on EP 8E
[1] Aborted transfer on EP 0F
[1] Aborted transfer on EP 8F

Device configured
Parsing Configuration descriptor (wTotalLength = 39)
[0:1] Interface 0: class = 255 subclass = 0 protocol = 0 is not supported
```