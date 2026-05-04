# HAL lib LVDS

Step 1 Select HV interface type

        TCON_LCD0->LCD_CTL_REG          0x040   80000120
        TCON_LCD0->LCD_HV_IF_REG        0x058   00000000

Step 2 Clock configuration

        CCU->PLL_PERI_CTRL_REG          0x020   c8216300
        CCU->PLL_VIDEO0_CTRL_REG        0x040   f8000f00
        CCU->TCONLCD_CLK_REG            0xB60   81000000
        TCON_LCD0->LCD_DCLK_REG         0x044   f0000007
        CCU->DSI_CLK_REG                0xB24   81000000

Step 3 Set sequence parameters

        TCON_LCD0->LCD_BASIC0_REG       0x048   025704ff
        TCON_LCD0->LCD_BASIC1_REG       0x04C   02b3002b
        TCON_LCD0->LCD_BASIC2_REG       0x050   0a340007
        TCON_LCD0->LCD_BASIC3_REG       0x054   00020002

Step 4 Open IO output

        TCON_LCD0->LCD_IO_TRI_REG       0x08C   00000000
        TCON_LCD0->LCD_IO_POL_REG       0x088   00000000

Step 5 LVDS digital logic configuration

        TCON_LCD0->LCD_LVDS_IF_REG      0x084   80100000
        TCON_LCD0->LVDS1_IF_REG         0x244   00000000

Step 6 LVDS controller configuration

PHY0 COMBO_PHY_REG

        DSI0_PHY->combo_phy_reg1        0x114   00000043
        DSI0_PHY->combo_phy_reg0        0x110   0000000f
        DSI0_PHY->dphy_ana4                     0x05c   00000000
        DSI0_PHY->dphy_ana3                     0x058   00000000
        DSI0_PHY->dphy_ana2                     0x054   00000000
        DSI0_PHY->dphy_ana1                     0x050   00000000

PHY1 LCD_LVDS0_ANA_REG

        TCON_LCD0->LCD_LVDS_ANA_REG[0]  0x220   00000000

Step 5-7 Set and open interrupt function

        TCON_LCD0->LCD_GINT0_REG        0x004   00008002
        TCON_LCD0->LCD_GINT1_REG        0x008   00000000

# RT LVDS

  Step 1 Select HV interface type

        TCON_LCD0->LCD_CTL_REG      0x040               800000a0
        TCON_LCD0->LCD_HV_IF_REG    0x058               00000000

Step 2 Clock configuration

        CCU->PLL_PERI_CTRL_REG      0x020               f8216300 N=100, M1=1, P0=2, P1=3 (1.2 Ghz)
        CCU->PLL_VIDEO0_CTRL_REG    0x040               f8000f00 N=16,  M1=1, M0=1 (video4x = 384 Mhz)
        CCU->TCONLCD_CLK_REG        0xB60               81000000 video4x 
        TCON_LCD0->LCD_DCLK_REG     0x044               f0000007 (54 Mhz)
        CCU->DSI_CLK_REG            0xB24               80000000

Step 3 Set sequence parameters

        TCON_LCD0->LCD_BASIC0_REG   0x048               01df04ff
        TCON_LCD0->LCD_BASIC1_REG   0x04C               055b002b
        TCON_LCD0->LCD_BASIC2_REG   0x050               04e40007
        TCON_LCD0->LCD_BASIC3_REG   0x054               00010001

Step 4 Open IO output

        TCON_LCD0->LCD_IO_TRI_REG   0x08C               00000000
        TCON_LCD0->LCD_IO_POL_REG   0x088               00000000

Step 5 LVDS digital logic configuration

        TCON_LCD0->LCD_LVDS_IF_REG  0x084               80100000
        TCON_LCD0->LVDS1_IF_REG     0x244               00100000

Step 6 LVDS controller configuration

PHY0 COMBO_PHY_REG

        DSI0_PHY->combo_phy_reg1    0x114               00000043
        DSI0_PHY->combo_phy_reg0    0x110               0000000f
        DSI0_PHY->dphy_ana4         0x05c               84000000
        DSI0_PHY->dphy_ana3         0x058               01040000
        DSI0_PHY->dphy_ana2         0x054               00000000
        DSI0_PHY->dphy_ana1         0x050               00000000

PHY1 LCD_LVDS0_ANA_REG

        TCON_LCD0->LCD_LVDS_ANA_REG[0]  0x220           00000000

Step 5-7 Set and open interrupt function

        TCON_LCD0->LCD_GINT0_REG    0x004           80002002
        TCON_LCD0->LCD_GINT1_REG    0x008           00000000

# HAL DSI

Step 1 Select HV interface type

        TCON_LCD0->LCD_CTL_REG          0x040   800000a0
        TCON_LCD0->LCD_HV_IF_REG        0x058   00000000

Step 2 Clock configuration

        CCU->PLL_PERI_CTRL_REG          0x020   c8216300
        CCU->PLL_VIDEO0_CTRL_REG        0x040   f8001a02
        CCU->TCONLCD_CLK_REG            0xB60   81000000
        TCON_LCD0->LCD_DCLK_REG         0x044   f0000004
        CCU->DSI_CLK_REG                0xB24   81000004

Step 3 Set sequence parameters

        TCON_LCD0->LCD_BASIC0_REG       0x048   01df04ff
        TCON_LCD0->LCD_BASIC1_REG       0x04C   02b50095
        TCON_LCD0->LCD_BASIC2_REG       0x050   0a38000b
        TCON_LCD0->LCD_BASIC3_REG       0x054   0028000a

Step 4 Open IO output

        TCON_LCD0->LCD_IO_TRI_REG       0x08C   00000000
        TCON_LCD0->LCD_IO_POL_REG       0x088   00000000

Step 5 LVDS digital logic configuration

        TCON_LCD0->LCD_LVDS_IF_REG      0x084   80100000
        TCON_LCD0->LVDS1_IF_REG         0x244   00000000

Step 6 LVDS controller configuration

PHY0 COMBO_PHY_REG

        DSI0_PHY->combo_phy_reg1                0x114   00000043
        DSI0_PHY->combo_phy_reg0                0x110   0000000b
        DSI0_PHY->dphy_ana4                     0x05c   844635ee
        DSI0_PHY->dphy_ana3                     0x058   ff040000
        DSI0_PHY->dphy_ana2                     0x054   0f000012
        DSI0_PHY->dphy_ana1                     0x050   80000000

PHY1 LCD_LVDS0_ANA_REG

        TCON_LCD0->LCD_LVDS_ANA_REG[0]  0x220   f1f80300

Step 5-7 Set and open interrupt function

        TCON_LCD0->LCD_GINT0_REG        0x004   00008002
        TCON_LCD0->LCD_GINT1_REG        0x008   00000000

# RT DSI 

Step 1 Select HV interface type

        TCON_LCD0->LCD_CTL_REG      0x040               81000000
        TCON_LCD0->LCD_HV_IF_REG    0x058               00000000

Step 2 Clock configuration

        CCU->PLL_PERI_CTRL_REG      0x020               f8216300 N=100, M1=1, P0=2, P1=3 (1.2 Ghz)
        CCU->PLL_VIDEO0_CTRL_REG    0x040               f8001a02 N=27,  M1=2, M0=1 (video4x = 324 Mhz)
        CCU->TCONLCD_CLK_REG        0xB60               81000000 video4x 
        TCON_LCD0->LCD_DCLK_REG     0x044               f0000004 (81 Mhz)
        CCU->DSI_CLK_REG            0xB24               81000004 Peri 600 / 5 = 120 Mhz

Step 3 Set sequence parameters

        TCON_LCD0->LCD_BASIC0_REG   0x048               01df04ff
        TCON_LCD0->LCD_BASIC1_REG   0x04C               00000000
        TCON_LCD0->LCD_BASIC2_REG   0x050               00000000
        TCON_LCD0->LCD_BASIC3_REG   0x054               00000000

Step 4 Open IO output

        TCON_LCD0->LCD_IO_TRI_REG   0x08C               00000000
        TCON_LCD0->LCD_IO_POL_REG   0x088               00000000

Step 5 LVDS digital logic configuration

        TCON_LCD0->LCD_LVDS_IF_REG  0x084               00000000
        TCON_LCD0->LVDS1_IF_REG     0x244               00000000

Step 6 LVDS controller configuration

PHY0 COMBO_PHY_REG

        DSI0_PHY->combo_phy_reg1    0x114               00000000
        DSI0_PHY->combo_phy_reg0    0x110               0000000b
        DSI0_PHY->dphy_ana4         0x05c               844635ee
        DSI0_PHY->dphy_ana3         0x058               ff040000
        DSI0_PHY->dphy_ana2         0x054               0f000012
        DSI0_PHY->dphy_ana1         0x050               80000000

PHY1 LCD_LVDS0_ANA_REG

        TCON_LCD0->LCD_LVDS_ANA_REG[0]  0x220           00000000

Step 5-7 Set and open interrupt function

        TCON_LCD0->LCD_GINT0_REG    0x004           00000a02
        TCON_LCD0->LCD_GINT1_REG    0x008           00000000