Plan:

1. TCON - need to check frequency from RTT
2. To found out how DSI dphy init in RTT/Linux. Is combo or lvds ?
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

DPHY init ?
