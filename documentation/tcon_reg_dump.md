# d1 lib with RGB

Step 1 Select HV interface type

        TCON_LCD0->LCD_CTL_REG                  800001f0
        TCON_LCD0->LCD_HV_IF_REG                00000000
Step 2 Clock configuration

        CCU->PLL_VIDEO0_CTRL_REG                f8000b03
        CCU->TCONLCD_CLK_REG                    81000000
        CCU->TCONLCD_BGR_REG                    00010001
        TCON_LCD0->LCD_DCLK_REG                 f000000c
        TCON_LCD0->LCD_DCLK_REG                 f000000c
        CCU->DSI_CLK_REG                        00000000
        CCU->DSI_BGR_REG                        00000000
        CCU->LVDS_BGR_REG                       00000000
Step 3 Set sequence parameters

        TCON_LCD0->LCD_BASIC0_REG               01df010f
        TCON_LCD0->LCD_BASIC1_REG               0263003b
        TCON_LCD0->LCD_BASIC2_REG               04100011
        TCON_LCD0->LCD_BASIC3_REG               000c0004
Step 4 Open IO output

        TCON_LCD0->LCD_IO_TRI_REG               00000000
        TCON_LCD0->LCD_IO_POL_REG               00000000
Step 5 LVDS digital logic configuration

        TCON_LCD0->LCD_LVDS_IF_REG              00000000
        TCON_LCD0->LVDS1_IF_REG                 00000000
Step 6 LVDS controller configuration

PHY0 COMBO_PHY_REG

        DSI0_PHY->combo_phy_reg1                00000000
        DSI0_PHY->combo_phy_reg0                00000000
        DSI0_PHY->dphy_ana4                     00000000
        DSI0_PHY->dphy_ana3                     00000000
        DSI0_PHY->dphy_ana2                     00000000
        DSI0_PHY->dphy_ana1                     00000000
PHY1 LCD_LVDS0_ANA_REG

        TCON_LCD0->LCD_LVDS_ANA_REG[0]          00000000

Step 5-7 Set and open interrupt function

        TCON_LCD0->LCD_GINT0_REG                20008003
        TCON_LCD0->LCD_GINT1_REG                00000000

# d1 lib with Devterm

Step 1 Select HV interface type

        TCON_LCD0->LCD_CTL_REG                  80000140
        TCON_LCD0->LCD_HV_IF_REG                00000000

Step 2 Clock configuration

        CCU->PLL_VIDEO0_CTRL_REG                f8003603
        CCU->TCONLCD_CLK_REG                    81000000
        CCU->TCONLCD_BGR_REG                    00010001
        TCON_LCD0->LCD_DCLK_REG                 f000000c
        TCON_LCD0->LCD_DCLK_REG                 f000000c
        CCU->DSI_CLK_REG                        82000003
        CCU->DSI_BGR_REG                        00010001
        CCU->LVDS_BGR_REG                       00010000

Step 3 Set sequence parameters

        TCON_LCD0->LCD_BASIC0_REG               01df04ff
        TCON_LCD0->LCD_BASIC1_REG               02b50095
        TCON_LCD0->LCD_BASIC2_REG               0a38000b
        TCON_LCD0->LCD_BASIC3_REG               0028000a

Step 4 Open IO output

        TCON_LCD0->LCD_IO_TRI_REG               00000000
        TCON_LCD0->LCD_IO_POL_REG               00000000

Step 5 LVDS digital logic configuration

        TCON_LCD0->LCD_LVDS_IF_REG              80100000
        TCON_LCD0->LVDS1_IF_REG                 80100000

Step 6 LVDS controller configuration

PHY0 COMBO_PHY_REG 

        DSI0_PHY->combo_phy_reg1                00000043
        DSI0_PHY->combo_phy_reg0                0000000f
        DSI0_PHY->dphy_ana4                     84000000
        DSI0_PHY->dphy_ana3                     01040000
        DSI0_PHY->dphy_ana2                     00000000
        DSI0_PHY->dphy_ana1                     00000000

PHY1 LCD_LVDS0_ANA_REG

        TCON_LCD0->LCD_LVDS_ANA_REG[0]          f1f80300

Step 5-7 Set and open interrupt function

        TCON_LCD0->LCD_GINT0_REG                20008003
        TCON_LCD0->LCD_GINT1_REG                00000000


# RTT with Devterm

Step 1 Select HV interface type

        TCON_LCD0->LCD_CTL_REG      0x040               81000000        31: LCD_EN 
                                                                        24: 8080 I/F
        TCON_LCD0->LCD_HV_IF_REG    0x058               00000000        28: 24-bit/1-cycle

Step 2 Clock configuration
        CCU->PLL_PERI_CTRL_REG      0x020               f8216300        31:PLL_EN
                                                                        30:PLL_LDO_EN    
                                                                        29:LOCK_ENABLE
                                                                        28:LOCK          
                                                                        27:GATE             
                                                                        21: P1 = 2+1
                                                                        16: P0 = 1+1
                                                                         8: N = 99+1
                                                                         0: M = 0+1

                                                                        PLL_PERI(2X) = 1.2Gz
                                                                        PLL_PERI(1X) = 600Mz
                                                                        PLL_PERI(800M) = 800Mz

        CCU->PLL_VIDEO0_CTRL_REG    0x040               f8001a02        31:PLL_EN
                                                                        30:PLL_LDO_EN             
                                                                        29:LOCK_ENABLE
                                                                        28:LOCK                   
                                                                        27:GATE
                                                                         8: N = 26+1
                                                                         1: M = 1+1

                                                                        PLL_VIDEO0(4X)= 324Mz
                                                                        PLL_VIDEO0(2X)= 162Mz
                                                                        PLL_VIDEO0(1X)= 81Mz


        CCU->TCONLCD_CLK_REG        0xB60               81000000        31: ON
                                                                        24: PLL_VIDEO0(4X) result
                                                                         8: N = 1
                                                                         0: M = 0+1

        CCU->TCONLCD_BGR_REG        0xB7C               00010001        Gate/reset

        TCON_LCD0->LCD_DCLK_REG     0x044               f0000004        28: dclk all
                                                                        0:  Div: 4

                                                                        81Mz?

        CCU->DSI_CLK_REG            0xB24               81000004        31: On
                                                                        24: PERI_1x
                                                                         0: 4+1     

                                                                        120Mz   

        CCU->DSI_BGR_REG            0xB4C               00010001        Gate/reset
        CCU->LVDS_BGR_REG           0xBAC               00000000        No

Step 3 Set sequence parameters

        TCON_LCD0->LCD_BASIC0_REG   0x048               01df04ff        H = 1279+1 , W = 479+1
        TCON_LCD0->LCD_BASIC1_REG   0x04C               00000000        0
        TCON_LCD0->LCD_BASIC2_REG   0x050               00000000        0
        TCON_LCD0->LCD_BASIC3_REG   0x054               00000000        0

Step 4 Open IO output

        TCON_LCD0->LCD_IO_TRI_REG   0x08C               00000000        0
        TCON_LCD0->LCD_IO_POL_REG   0x088               00000000        0

Step 5 LVDS digital logic configuration

        TCON_LCD0->LCD_LVDS_IF_REG  0x084               00000000        0
        TCON_LCD0->LVDS1_IF_REG     0x244               00000000        0

Step 6 LVDS controller configuration

PHY0 COMBO_PHY_REG

        DSI0_PHY->combo_phy_reg1    0x114               00000000
        DSI0_PHY->combo_phy_reg0    0x110               0000000b
        DSI0_PHY->dphy_ana4         0x05c               844635ee
        DSI0_PHY->dphy_ana3         0x058               ff040000
        DSI0_PHY->dphy_ana2         0x054               0f000012
        DSI0_PHY->dphy_ana1         0x050               80000000

PHY1 LCD_LVDS0_ANA_REG

        TCON_LCD0->LCD_LVDS_ANA_REG[0]  0x220           00000000        0

Step 5-7 Set and open interrupt function

        TCON_LCD0->LCD_GINT0_REG    0x004               00000a02        11:
                                                                         9:
                                                                         1:
        TCON_LCD0->LCD_GINT1_REG    0x008               00000000


# RTT with uConsole

# RTT with RGB



