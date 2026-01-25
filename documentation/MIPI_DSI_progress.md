Plan:

1. TCON - need to check frequency from RTT
2. To found out how DSI dphy init in RTT/Linux.
3. DSI stack - check register addresses
4. DSI stack - long short package to check with RTT/Linux

sunxi-linux\drivers\gpu\drm\sun4i\sun4i_tcon.c

sun20i_d1_lcd_quirks
.has_channel_0	= true,
.dclk_min_div	= 1,
.set_mux		= sun8i_r40_tcon_tv_set_mux,


sun4i_tcon_bind
sun4i_dclk_create
sun4i_crtc_init
sun4i_rgb_init
sun4i_tcon0_mode_set_cpu


DISPLAY_TOP 0x05460000---0x05460FFF

dsi: dsi@5450000 {
			compatible = "allwinner,sun20i-d1-mipi-dsi",
				     "allwinner,sun50i-a100-mipi-dsi";

sunxi-linux\drivers\phy\allwinner\phy-sun6i-mipi-dphy.c


SUN50I_DPHY_PLL_REG0_CP36_EN	23
SUN50I_DPHY_PLL_REG0_LDO_EN		22
SUN50I_DPHY_PLL_REG0_EN_LVS		21
SUN50I_DPHY_PLL_REG0_PLL_EN		20
SUN50I_DPHY_PLL_REG0_P(n)		16
SUN50I_DPHY_PLL_REG0_N(n)		8
SUN50I_DPHY_PLL_REG0_NDET		7
SUN50I_DPHY_PLL_REG0_TDIV		6
SUN50I_DPHY_PLL_REG0_M0(n)		4
SUN50I_DPHY_PLL_REG0_M1(n)		1


frq = dclk * 24 / 4 * 1000000


Clk = 24 MHz*n/(p+1)/(m0+1)


HAL: 	SUN50I_DPHY_PLL_REG0		0x104		00f00e82
        CCU->PLL_PERI_CTRL_REG      0x020       c8216300
        CCU->PLL_VIDEO0_CTRL_REG    0x040       f8001a03
        TCON_LCD0->LCD_DCLK_REG     0x044       f0000004
        CCU->DSI_CLK_REG            0xB24       81000004

M1 =2, M0 = 0, DIV =0, DET = 1, N = 14, P = 0
Clk = 14 Mhz


RTT: 	SUN50I_DPHY_PLL_REG0		0x104		00f76e82
        CCU->PLL_PERI_CTRL_REG      0x020       f8216300
        CCU->PLL_VIDEO0_CTRL_REG    0x040       f8001a02
        TCON_LCD0->LCD_DCLK_REG     0x044       f0000004
        CCU->DSI_CLK_REG            0xB24       81000004

M1 =2, M0 = 0, DIV =0, DET = 1, N = 110, P = 7
Clk = 330 Mhz


Shenzhen Tianxianwei Technology 

TTW686VVC-01 181x66.6x4.5

Devterm:
TXV686001/1 202205200778 181x66.5x4.5 MIPI ICNL9707

Txw site:
TXW686005B0 66.7*181*4.8 MIPI ICNL9707
TXW686003B0 66.7*181*4.8 LVDS NV3051F1

Ali:
TXW686013B0 66.7*181.2*4.4 RGB NV3052C
TXW686017B0 66.7*181.2*4.4 LVDS NV3051F1
TXW686003B0 66.7*181*4.8 LVDS NV3051F1
TXW686005B0 66.7*181*4.8 MIPI  ICNL9707