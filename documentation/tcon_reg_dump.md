# hal lib LVDS

        CCU->PLL_PERI_CTRL_REG          0x020           c8216300
        CCU->PLL_VIDEO0_CTRL_REG        0x040           f8001f01

        N = 32
        M1 = 2

        CCU->TCONLCD_CLK_REG            0xB60           81000000
        CCU->TCONLCD_BGR_REG            0xB7C           00010001
        TCON_LCD0->LCD_DCLK_REG         0x044           f0000007
        CCU->DSI_CLK_REG                0xB24           81000003

        PLL_PERI
        M = 4

        CCU->DSI_BGR_REG                0xB4C           00010001
        CCU->LVDS_BGR_REG               0xBAC           00010000

# RT LVDS

        CCU->PLL_PERI_CTRL_REG      0x020               f8216300
        CCU->PLL_VIDEO0_CTRL_REG    0x040               f8000f00

        N = 16
        M1 = 1


        CCU->TCONLCD_CLK_REG        0xB60               81000000
        CCU->TCONLCD_BGR_REG        0xB7C               00010001
        TCON_LCD0->LCD_DCLK_REG     0x044               f0000007
        CCU->DSI_CLK_REG            0xB24               80000000

        HOSC
        M = 1

        CCU->DSI_BGR_REG            0xB4C               00010001
        CCU->LVDS_BGR_REG           0xBAC               00010000