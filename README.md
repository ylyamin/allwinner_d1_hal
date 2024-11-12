# TODO:


- gcc flags
- irq implementation. eps after handle_interrupt ?
- ccu, pll0, usb_clk
- MMU ?
- FreeRTOS

- long make, 'all' not automatic
- repo structure and docs
- changelog
- hftrx to learn




# Reuse:

aw_f133 aw_f133_app.ld crt_RV64IFDV.S from https://github.com/ua1arn/hftrx/tree/develop-linux/arch/aw_f133
tinyusb-ohci aw_lib from https://github.com/robots/allwinner_t113/tree/master/common/aw
irq-d1.c exeptions from https://github.com/xboot/xboot
memcpy.s memset.S from https://github.com/xboot/xfel/tree/master/payloads/d1_f133/d1-ddr/source


# Results:

CACHE
[0:0] Get Descriptor: 80 06 00 01 00 00 08 00

NO CACHE:
usbh_control_xfer_cb
"[0:0] Control STALLED"