/**
 * p-boot - pico sized bootloader
 *
 * Copyright (C) 2020  Ondřej Jirman <megi@xff.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Based on u-boot code: (GPL-2.0+)
 * sunxi_de2.c: (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 * lcdc.c: (C) Copyright 2013-2014 Luc Verhaegen <libv@skynet.be>
 * lcdc.c: (C) Copyright 2014-2015 Hans de Goede <hdegoede@redhat.com>
 * lcdc.c: (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 * sunxi_lcd.c: (C) Copyright 2017 Vasily Khoruzhick <anarsoul@gmail.com>
 * Based on Linux code:
 * sun6i_mipi_dsi.c: (GPL-2.0+)
 *  Copyright (c) 2016 Allwinnertech Co., Ltd.
 *  Copyright (C) 2017-2018 Bootlin / Maxime Ripard <maxime.ripard@bootlin.com>
 * panel-xingbangda-xbd599.c: (GPL-2.0+)
 *  Copyright (c) 2019 Icenowy Zheng <icenowy@aosc.io>
 */

#include <common.h>
#include <malloc.h>
#include <arm_neon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/display2.h>
#include <asm/arch/dram.h>
#include <asm/arch/gpio.h>
#include <asm/arch/lcdc.h>
#include <asm/arch/prcm.h>
#include <asm/arch/pwm.h>
#include <asm/arch/spl.h>
#include <asm/armv8/mmu.h>
#include <asm-generic/gpio.h>
#include <asm/system.h>
#include <cpu_func.h>
#include "mmu.h"
#include "pmic.h"
#include "ccu.h"
#include "display.h"

#ifndef DUMP_DSI_INIT
#define DUMP_DSI_INIT 0
#endif

#ifndef DSI_FULL_INIT
#define DSI_FULL_INIT 0
#endif

#ifndef DE2_RESIZE
#define DE2_RESIZE 0
#endif

//
// General display enablement flow:
//
// * power up regulators
// * enable/setup clocks/PLLs
//   * PLL_VIDEO0
//   * DCLK
//   * MIPI_DSI
//   * DE
// * enable dphy
// * enable backlight
// * configure tcon
// * configure DE2
// * enable dsi host controller
// * run dsi commands on the panel to initialize it/turn it on
//

// {{{ PANEL

#define PANEL_HDISPLAY		(720)
#define PANEL_HSYNC_START	(720 + 30)
#define PANEL_HSYNC_END		(720 + 30 + 28)
#define PANEL_HTOTAL		(720 + 30 + 28 + 30)
#define PANEL_VDISPLAY		(1440)
#define PANEL_VSYNC_START	(1440 + 18)
#define PANEL_VSYNC_END		(1440 + 18 + 10)
#define PANEL_VTOTAL		(1440 + 18 + 10 + 17)
//#define PANEL_CLOCK		(PANEL_HTOTAL * PANEL_VTOTAL * 60 / 1000)
#define PANEL_CLOCK		(72000)
#define PANEL_LANES		4
#define PANEL_DSI_BPP		24
#define PANEL_BURST		0

/*
 * Init sequence was supplied by the panel vendor:
 */

/* Manufacturer specific Commands send via DSI */
#define ST7703_CMD_ALL_PIXEL_OFF 0x22
#define ST7703_CMD_ALL_PIXEL_ON  0x23
#define ST7703_CMD_SETDISP       0xB2
#define ST7703_CMD_SETRGBIF      0xB3
#define ST7703_CMD_SETCYC        0xB4
#define ST7703_CMD_SETBGP        0xB5
#define ST7703_CMD_SETVCOM       0xB6
#define ST7703_CMD_SETOTP        0xB7
#define ST7703_CMD_SETPOWER_EXT  0xB8
#define ST7703_CMD_SETEXTC       0xB9
#define ST7703_CMD_SETMIPI       0xBA
#define ST7703_CMD_SETVDC        0xBC
#define ST7703_CMD_SETSCR        0xC0
#define ST7703_CMD_SETPOWER      0xC1
#define ST7703_CMD_UNK_C6        0xC6
#define ST7703_CMD_SETPANEL      0xCC
#define ST7703_CMD_SETGAMMA      0xE0
#define ST7703_CMD_SETEQ         0xE3
#define ST7703_CMD_SETGIP1       0xE9
#define ST7703_CMD_SETGIP2       0xEA

#define MIPI_DCS_SET_DISPLAY_ON	 0x29
#define MIPI_DCS_EXIT_SLEEP_MODE 0x11

struct dcs_seq {
	u8 len;
	const u8 *data;
	u8 type;
};

#define dcs_seq_data(cmd, data...) \
	static const u8 panel_dcs_initlist_data_##cmd[] = { cmd, data };
#define dcs_seq_desc(cmd, data...) \
	{ sizeof(panel_dcs_initlist_data_##cmd), panel_dcs_initlist_data_##cmd },

dcs_seq_data(ST7703_CMD_SETEXTC,
	     0xF1, 0x12, 0x83)
dcs_seq_data(ST7703_CMD_SETMIPI,
	     0x33, 0x81, 0x05, 0xF9, 0x0E, 0x0E, 0x20, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x25,
	     0x00, 0x91, 0x0a, 0x00, 0x00, 0x02, 0x4F, 0x11,
	     0x00, 0x00, 0x37);
dcs_seq_data(ST7703_CMD_SETPOWER_EXT,
	     0x25, 0x22, 0x20, 0x03)
dcs_seq_data(ST7703_CMD_SETRGBIF,
	     0x10, 0x10, 0x05, 0x05, 0x03, 0xFF, 0x00, 0x00,
	     0x00, 0x00)

dcs_seq_data(ST7703_CMD_SETSCR,
	     0x73, 0x73, 0x50, 0x50, 0x00, 0xC0, 0x08, 0x70,
	     0x00)
dcs_seq_data(ST7703_CMD_SETVDC, 0x4E)
dcs_seq_data(ST7703_CMD_SETPANEL, 0x0B)
dcs_seq_data(ST7703_CMD_SETCYC, 0x80)
dcs_seq_data(ST7703_CMD_SETDISP, 0xF0, 0x12, 0xF0)
dcs_seq_data(ST7703_CMD_SETEQ,
	     0x00, 0x00, 0x0B, 0x0B, 0x10, 0x10, 0x00, 0x00,
	     0x00, 0x00, 0xFF, 0x00, 0xC0, 0x10)
dcs_seq_data(0xC6, 0x01, 0x00, 0xFF, 0xFF, 0x00)
dcs_seq_data(ST7703_CMD_SETPOWER,
	     0x74, 0x00, 0x32, 0x32, 0x77, 0xF1, 0xFF, 0xFF,
	     0xCC, 0xCC, 0x77, 0x77)
dcs_seq_data(ST7703_CMD_SETBGP, 0x07, 0x07)
dcs_seq_data(ST7703_CMD_SETVCOM, 0x2C, 0x2C)
dcs_seq_data(0xBF, 0x02, 0x11, 0x00)

dcs_seq_data(ST7703_CMD_SETGIP1,
	     0x82, 0x10, 0x06, 0x05, 0xA2, 0x0A, 0xA5, 0x12,
	     0x31, 0x23, 0x37, 0x83, 0x04, 0xBC, 0x27, 0x38,
	     0x0C, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C, 0x00,
	     0x03, 0x00, 0x00, 0x00, 0x75, 0x75, 0x31, 0x88,
	     0x88, 0x88, 0x88, 0x88, 0x88, 0x13, 0x88, 0x64,
	     0x64, 0x20, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
	     0x02, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
dcs_seq_data(ST7703_CMD_SETGIP2,
	     0x02, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x02, 0x46, 0x02, 0x88,
	     0x88, 0x88, 0x88, 0x88, 0x88, 0x64, 0x88, 0x13,
	     0x57, 0x13, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
	     0x75, 0x88, 0x23, 0x14, 0x00, 0x00, 0x02, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0A,
	     0xA5, 0x00, 0x00, 0x00, 0x00)
dcs_seq_data(ST7703_CMD_SETGAMMA,
	     0x00, 0x09, 0x0D, 0x23, 0x27, 0x3C, 0x41, 0x35,
	     0x07, 0x0D, 0x0E, 0x12, 0x13, 0x10, 0x12, 0x12,
	     0x18, 0x00, 0x09, 0x0D, 0x23, 0x27, 0x3C, 0x41,
	     0x35, 0x07, 0x0D, 0x0E, 0x12, 0x13, 0x10, 0x12,
	     0x12, 0x18)
dcs_seq_data(MIPI_DCS_EXIT_SLEEP_MODE)
dcs_seq_data(MIPI_DCS_SET_DISPLAY_ON)

static const struct dcs_seq panel_dcs_seq_initlist[] = {
	dcs_seq_desc(ST7703_CMD_SETEXTC)
	dcs_seq_desc(ST7703_CMD_SETMIPI)
	dcs_seq_desc(ST7703_CMD_SETPOWER_EXT)
	dcs_seq_desc(ST7703_CMD_SETRGBIF)
	dcs_seq_desc(ST7703_CMD_SETSCR)
	dcs_seq_desc(ST7703_CMD_SETVDC)
	dcs_seq_desc(ST7703_CMD_SETPANEL)
	dcs_seq_desc(ST7703_CMD_SETCYC)
	dcs_seq_desc(ST7703_CMD_SETDISP)
	dcs_seq_desc(ST7703_CMD_SETEQ)
	dcs_seq_desc(0xC6)
	dcs_seq_desc(ST7703_CMD_SETPOWER)
	dcs_seq_desc(ST7703_CMD_SETBGP)
	dcs_seq_desc(ST7703_CMD_SETVCOM)
	dcs_seq_desc(0xBF)
	dcs_seq_desc(ST7703_CMD_SETGIP1)
	dcs_seq_desc(ST7703_CMD_SETGIP2)
	dcs_seq_desc(ST7703_CMD_SETGAMMA)
	dcs_seq_desc(MIPI_DCS_EXIT_SLEEP_MODE)
	{ .type = 1, .len = 120 },
	dcs_seq_desc(MIPI_DCS_SET_DISPLAY_ON)
};

ssize_t mipi_dsi_dcs_write(const u8 *data, size_t len);

static void panel_init(void)
{
	int i, ret;

	// run panel init

	for (i = 0; i < ARRAY_SIZE(panel_dcs_seq_initlist); i++) {
		const struct dcs_seq* s = &panel_dcs_seq_initlist[i];

		if (s->type) {
			udelay(s->len * 1000);
			continue;
		}

                ret = mipi_dsi_dcs_write(s->data, s->len);
                if (ret < 0) {
			printf("DCS failed\n");
                        return;
		}
	}
}

// }}}
// {{{ DPHY

#define DPHY_GCTL_REG             0x00
#define DPHY_GCTL_LANE_NUM(n)             ((((n) - 1) & 3) << 4)
#define DPHY_GCTL_EN                      BIT(0)

#define DPHY_TX_CTL_REG           0x04
#define DPHY_TX_CTL_HS_TX_CLK_CONT        BIT(28)

#define DPHY_TX_TIME0_REG         0x10
#define DPHY_TX_TIME0_HS_TRAIL(n)         (((n) & 0xff) << 24)
#define DPHY_TX_TIME0_HS_PREPARE(n)       (((n) & 0xff) << 16)
#define DPHY_TX_TIME0_LP_CLK_DIV(n)       ((n) & 0xff)

#define DPHY_TX_TIME1_REG         0x14
#define DPHY_TX_TIME1_CLK_POST(n)         (((n) & 0xff) << 24)
#define DPHY_TX_TIME1_CLK_PRE(n)          (((n) & 0xff) << 16)
#define DPHY_TX_TIME1_CLK_ZERO(n)         (((n) & 0xff) << 8)
#define DPHY_TX_TIME1_CLK_PREPARE(n)      ((n) & 0xff)

#define DPHY_TX_TIME2_REG         0x18
#define DPHY_TX_TIME2_CLK_TRAIL(n)        ((n) & 0xff)

#define DPHY_TX_TIME3_REG         0x1c

#define DPHY_TX_TIME4_REG         0x20
#define DPHY_TX_TIME4_HS_TX_ANA1(n)       (((n) & 0xff) << 8)
#define DPHY_TX_TIME4_HS_TX_ANA0(n)       ((n) & 0xff)

#define DPHY_ANA0_REG             0x4c
#define DPHY_ANA0_REG_PWS                 BIT(31)
#define DPHY_ANA0_REG_DMPC                BIT(28)
#define DPHY_ANA0_REG_DMPD(n)             (((n) & 0xf) << 24)
#define DPHY_ANA0_REG_SLV(n)              (((n) & 7) << 12)
#define DPHY_ANA0_REG_DEN(n)              (((n) & 0xf) << 8)

#define DPHY_ANA1_REG             0x50
#define DPHY_ANA1_REG_VTTMODE             BIT(31)
#define DPHY_ANA1_REG_CSMPS(n)            (((n) & 3) << 28)
#define DPHY_ANA1_REG_SVTT(n)             (((n) & 0xf) << 24)

#define DPHY_ANA2_REG             0x54
#define DPHY_ANA2_EN_P2S_CPU(n)           (((n) & 0xf) << 24)
#define DPHY_ANA2_EN_P2S_CPU_MASK         GENMASK(27, 24)
#define DPHY_ANA2_EN_CK_CPU               BIT(4)
#define DPHY_ANA2_REG_ENIB                BIT(1)

#define DPHY_ANA3_REG             0x58
#define DPHY_ANA3_EN_VTTD(n)              (((n) & 0xf) << 28)
#define DPHY_ANA3_EN_VTTD_MASK            GENMASK(31, 28)
#define DPHY_ANA3_EN_VTTC                 BIT(27)
#define DPHY_ANA3_EN_DIV                  BIT(26)
#define DPHY_ANA3_EN_LDOC                 BIT(25)
#define DPHY_ANA3_EN_LDOD                 BIT(24)
#define DPHY_ANA3_EN_LDOR                 BIT(18)

#define DPHY_ANA4_REG             0x5c
#define DPHY_ANA4_REG_DMPLVC              BIT(24)
#define DPHY_ANA4_REG_DMPLVD(n)           (((n) & 0xf) << 20)
#define DPHY_ANA4_REG_CKDV(n)             (((n) & 0x1f) << 12)
#define DPHY_ANA4_REG_TMSC(n)             (((n) & 3) << 10)
#define DPHY_ANA4_REG_TMSD(n)             (((n) & 3) << 8)
#define DPHY_ANA4_REG_TXDNSC(n)           (((n) & 3) << 6)
#define DPHY_ANA4_REG_TXDNSD(n)           (((n) & 3) << 4)
#define DPHY_ANA4_REG_TXPUSC(n)           (((n) & 3) << 2)
#define DPHY_ANA4_REG_TXPUSD(n)           ((n) & 3)

#define DPHY_DBG5_REG             0xf4

#define DPHY_BASE 0x01ca1000u

noinline
static void dphy_write(unsigned long reg, uint32_t val)
{
	writel(val, DPHY_BASE + reg);
}

noinline
static void dphy_update_bits(unsigned long reg, uint32_t mask, uint32_t val)
{
	uint32_t tmp = readl(DPHY_BASE + reg);
	tmp &= ~mask;
	writel(tmp | val, DPHY_BASE + reg);
}

static void dphy_enable(void)
{
        u8 lanes_mask = GENMASK(PANEL_LANES - 1, 0);

	// 150MHz (600 / 4)
	writel((1 << 15 /* enable */) |
	       (2 << 8 /* src: 0=video0(1x) 2=periph0(1x) */) |
	       ((4 - 1) << 0) /* M-1 */,
	       CCU_MIPI_DSI_CLK);

	dphy_write(DPHY_TX_CTL_REG,
		   DPHY_TX_CTL_HS_TX_CLK_CONT);

	dphy_write(DPHY_TX_TIME0_REG,
		   DPHY_TX_TIME0_LP_CLK_DIV(14) |
		   DPHY_TX_TIME0_HS_PREPARE(6) |
		   DPHY_TX_TIME0_HS_TRAIL(10));

	dphy_write(DPHY_TX_TIME1_REG,
		   DPHY_TX_TIME1_CLK_PREPARE(7) |
		   DPHY_TX_TIME1_CLK_ZERO(50) |
		   DPHY_TX_TIME1_CLK_PRE(3) |
		   DPHY_TX_TIME1_CLK_POST(10));

	dphy_write(DPHY_TX_TIME2_REG,
		   DPHY_TX_TIME2_CLK_TRAIL(30));

	dphy_write(DPHY_TX_TIME3_REG, 0);

	dphy_write(DPHY_TX_TIME4_REG,
		   DPHY_TX_TIME4_HS_TX_ANA0(3) |
		   DPHY_TX_TIME4_HS_TX_ANA1(3));

	dphy_write(DPHY_GCTL_REG,
		   DPHY_GCTL_LANE_NUM(PANEL_LANES) |
		   DPHY_GCTL_EN);

	dphy_write(DPHY_ANA0_REG,
		   DPHY_ANA0_REG_PWS |
		   DPHY_ANA0_REG_DMPC |
		   DPHY_ANA0_REG_SLV(7) |
		   DPHY_ANA0_REG_DMPD(lanes_mask) |
		   DPHY_ANA0_REG_DEN(lanes_mask));

	dphy_write(DPHY_ANA1_REG,
		   DPHY_ANA1_REG_CSMPS(1) |
		   DPHY_ANA1_REG_SVTT(7));

	dphy_write(DPHY_ANA4_REG,
		   DPHY_ANA4_REG_CKDV(1) |
		   DPHY_ANA4_REG_TMSC(1) |
		   DPHY_ANA4_REG_TMSD(1) |
		   DPHY_ANA4_REG_TXDNSC(1) |
		   DPHY_ANA4_REG_TXDNSD(1) |
		   DPHY_ANA4_REG_TXPUSC(1) |
		   DPHY_ANA4_REG_TXPUSD(1) |
		   DPHY_ANA4_REG_DMPLVC |
		   DPHY_ANA4_REG_DMPLVD(lanes_mask));

	dphy_write(DPHY_ANA2_REG,
		   DPHY_ANA2_REG_ENIB);
	udelay(5);

	dphy_write(DPHY_ANA3_REG,
		   DPHY_ANA3_EN_LDOR |
		   DPHY_ANA3_EN_LDOC |
		   DPHY_ANA3_EN_LDOD);
	udelay(1);

	dphy_update_bits(DPHY_ANA3_REG,
			 DPHY_ANA3_EN_VTTC |
			 DPHY_ANA3_EN_VTTD_MASK,
			 DPHY_ANA3_EN_VTTC |
			 DPHY_ANA3_EN_VTTD(lanes_mask));
	udelay(1);

	dphy_update_bits(DPHY_ANA3_REG,
			 DPHY_ANA3_EN_DIV,
			 DPHY_ANA3_EN_DIV);
	udelay(1);

	dphy_update_bits(DPHY_ANA2_REG,
			 DPHY_ANA2_EN_CK_CPU,
			 DPHY_ANA2_EN_CK_CPU);
	udelay(1);

	dphy_update_bits(DPHY_ANA1_REG,
			 DPHY_ANA1_REG_VTTMODE,
			 DPHY_ANA1_REG_VTTMODE);

	dphy_update_bits(DPHY_ANA2_REG,
			 DPHY_ANA2_EN_P2S_CPU_MASK,
			 DPHY_ANA2_EN_P2S_CPU(lanes_mask));
}

// }}}
// {{{ DSI

#define SUN6I_DSI_CTL_REG               0x000
#define SUN6I_DSI_CTL_EN                        BIT(0)

#define SUN6I_DSI_BASIC_CTL_REG         0x00c
#define SUN6I_DSI_BASIC_CTL_TRAIL_INV(n)                (((n) & 0xf) << 4)
#define SUN6I_DSI_BASIC_CTL_TRAIL_FILL          BIT(3)
#define SUN6I_DSI_BASIC_CTL_HBP_DIS             BIT(2)
#define SUN6I_DSI_BASIC_CTL_HSA_HSE_DIS         BIT(1)
#define SUN6I_DSI_BASIC_CTL_VIDEO_BURST         BIT(0)

#define SUN6I_DSI_BASIC_CTL0_REG        0x010
#define SUN6I_DSI_BASIC_CTL0_HS_EOTP_EN         BIT(18)
#define SUN6I_DSI_BASIC_CTL0_CRC_EN             BIT(17)
#define SUN6I_DSI_BASIC_CTL0_ECC_EN             BIT(16)
#define SUN6I_DSI_BASIC_CTL0_INST_ST            BIT(0)

#define SUN6I_DSI_BASIC_CTL1_REG        0x014
#define SUN6I_DSI_BASIC_CTL1_VIDEO_ST_DELAY(n)  (((n) & 0x1fff) << 4)
#define SUN6I_DSI_BASIC_CTL1_VIDEO_FILL         BIT(2)
#define SUN6I_DSI_BASIC_CTL1_VIDEO_PRECISION    BIT(1)
#define SUN6I_DSI_BASIC_CTL1_VIDEO_MODE         BIT(0)

#define SUN6I_DSI_BASIC_SIZE0_REG       0x018
#define SUN6I_DSI_BASIC_SIZE0_VBP(n)            (((n) & 0xfff) << 16)
#define SUN6I_DSI_BASIC_SIZE0_VSA(n)            ((n) & 0xfff)

#define SUN6I_DSI_BASIC_SIZE1_REG       0x01c
#define SUN6I_DSI_BASIC_SIZE1_VT(n)             (((n) & 0xfff) << 16)
#define SUN6I_DSI_BASIC_SIZE1_VACT(n)           ((n) & 0xfff)

#define SUN6I_DSI_INST_FUNC_REG(n)      (0x020 + (n) * 0x04)
#define SUN6I_DSI_INST_FUNC_INST_MODE(n)        (((n) & 0xf) << 28)
#define SUN6I_DSI_INST_FUNC_ESCAPE_ENTRY(n)     (((n) & 0xf) << 24)
#define SUN6I_DSI_INST_FUNC_TRANS_PACKET(n)     (((n) & 0xf) << 20)
#define SUN6I_DSI_INST_FUNC_LANE_CEN            BIT(4)
#define SUN6I_DSI_INST_FUNC_LANE_DEN(n)         ((n) & 0xf)

#define SUN6I_DSI_INST_LOOP_SEL_REG     0x040

#define SUN6I_DSI_INST_LOOP_NUM_REG(n)  (0x044 + (n) * 0x10)
#define SUN6I_DSI_INST_LOOP_NUM_N1(n)           (((n) & 0xfff) << 16)
#define SUN6I_DSI_INST_LOOP_NUM_N0(n)           ((n) & 0xfff)

#define SUN6I_DSI_INST_JUMP_SEL_REG     0x048

#define SUN6I_DSI_INST_JUMP_CFG_REG(n)  (0x04c + (n) * 0x04)
#define SUN6I_DSI_INST_JUMP_CFG_TO(n)           (((n) & 0xf) << 20)
#define SUN6I_DSI_INST_JUMP_CFG_POINT(n)        (((n) & 0xf) << 16)
#define SUN6I_DSI_INST_JUMP_CFG_NUM(n)          ((n) & 0xffff)

#define SUN6I_DSI_TRANS_START_REG       0x060

#define SUN6I_DSI_TRANS_ZERO_REG        0x078

#define SUN6I_DSI_TCON_DRQ_REG          0x07c
#define SUN6I_DSI_TCON_DRQ_ENABLE_MODE          BIT(28)
#define SUN6I_DSI_TCON_DRQ_SET(n)               ((n) & 0x3ff)

#define SUN6I_DSI_PIXEL_CTL0_REG        0x080
#define SUN6I_DSI_PIXEL_CTL0_PD_PLUG_DISABLE    BIT(16)
#define SUN6I_DSI_PIXEL_CTL0_FORMAT(n)          ((n) & 0xf)

#define SUN6I_DSI_PIXEL_CTL1_REG        0x084

#define SUN6I_DSI_PIXEL_PH_REG          0x090
#define SUN6I_DSI_PIXEL_PH_ECC(n)               (((n) & 0xff) << 24)
#define SUN6I_DSI_PIXEL_PH_WC(n)                (((n) & 0xffff) << 8)
#define SUN6I_DSI_PIXEL_PH_VC(n)                (((n) & 3) << 6)
#define SUN6I_DSI_PIXEL_PH_DT(n)                ((n) & 0x3f)

#define SUN6I_DSI_PIXEL_PF0_REG         0x098
#define SUN6I_DSI_PIXEL_PF0_CRC_FORCE(n)        ((n) & 0xffff)

#define SUN6I_DSI_PIXEL_PF1_REG         0x09c
#define SUN6I_DSI_PIXEL_PF1_CRC_INIT_LINEN(n)   (((n) & 0xffff) << 16)
#define SUN6I_DSI_PIXEL_PF1_CRC_INIT_LINE0(n)   ((n) & 0xffff)

#define SUN6I_DSI_SYNC_HSS_REG          0x0b0

#define SUN6I_DSI_SYNC_HSE_REG          0x0b4

#define SUN6I_DSI_SYNC_VSS_REG          0x0b8

#define SUN6I_DSI_SYNC_VSE_REG          0x0bc

#define SUN6I_DSI_BLK_HSA0_REG          0x0c0

#define SUN6I_DSI_BLK_HSA1_REG          0x0c4
#define SUN6I_DSI_BLK_PF(n)                     (((n) & 0xffff) << 16)
#define SUN6I_DSI_BLK_PD(n)                     ((n) & 0xff)

#define SUN6I_DSI_BLK_HBP0_REG          0x0c8

#define SUN6I_DSI_BLK_HBP1_REG          0x0cc

#define SUN6I_DSI_BLK_HFP0_REG          0x0d0

#define SUN6I_DSI_BLK_HFP1_REG          0x0d4

#define SUN6I_DSI_BLK_HBLK0_REG         0x0e0

#define SUN6I_DSI_BLK_HBLK1_REG         0x0e4

#define SUN6I_DSI_BLK_VBLK0_REG         0x0e8

#define SUN6I_DSI_BLK_VBLK1_REG         0x0ec

#define SUN6I_DSI_BURST_LINE_REG        0x0f0
#define SUN6I_DSI_BURST_LINE_SYNC_POINT(n)      (((n) & 0xffff) << 16)
#define SUN6I_DSI_BURST_LINE_NUM(n)             ((n) & 0xffff)

#define SUN6I_DSI_BURST_DRQ_REG         0x0f4
#define SUN6I_DSI_BURST_DRQ_EDGE1(n)            (((n) & 0xffff) << 16)
#define SUN6I_DSI_BURST_DRQ_EDGE0(n)            ((n) & 0xffff)

#define SUN6I_DSI_CMD_CTL_REG           0x200
#define SUN6I_DSI_CMD_CTL_RX_OVERFLOW           BIT(26)
#define SUN6I_DSI_CMD_CTL_RX_FLAG               BIT(25)
#define SUN6I_DSI_CMD_CTL_TX_FLAG               BIT(9)

#define SUN6I_DSI_CMD_RX_REG(n)         (0x240 + (n) * 0x04)

#define SUN6I_DSI_DEBUG_DATA_REG        0x2f8

#define SUN6I_DSI_CMD_TX_REG(n)         (0x300 + (n) * 0x04)

#define SUN6I_DSI_SYNC_POINT            40

#define DSI_BASE 0x01ca0000u

#define MIPI_DSI_BLANKING_PACKET 0x19
#define MIPI_DSI_PACKED_PIXEL_STREAM_24	0x3e
#define MIPI_DSI_V_SYNC_START	 0x01
#define MIPI_DSI_V_SYNC_END	 0x11
#define MIPI_DSI_H_SYNC_START	 0x21
#define MIPI_DSI_H_SYNC_END	 0x31
#define MIPI_DSI_DCS_LONG_WRITE	       0x39
#define MIPI_DSI_DCS_SHORT_WRITE_PARAM 0x15
#define MIPI_DSI_DCS_SHORT_WRITE       0x05

enum sun6i_dsi_start_inst {
        DSI_START_LPRX,
        DSI_START_LPTX,
        DSI_START_HSC,
        DSI_START_HSD,
};

enum sun6i_dsi_inst_id {
        DSI_INST_ID_LP11        = 0,
        DSI_INST_ID_TBA,
        DSI_INST_ID_HSC,
        DSI_INST_ID_HSD,
        DSI_INST_ID_LPDT,
        DSI_INST_ID_HSCEXIT,
        DSI_INST_ID_NOP,
        DSI_INST_ID_DLY,
        DSI_INST_ID_END         = 15,
};

enum sun6i_dsi_inst_mode {
        DSI_INST_MODE_STOP      = 0,
        DSI_INST_MODE_TBA,
        DSI_INST_MODE_HS,
        DSI_INST_MODE_ESCAPE,
        DSI_INST_MODE_HSCEXIT,
        DSI_INST_MODE_NOP,
};

enum sun6i_dsi_inst_escape {
        DSI_INST_ESCA_LPDT      = 0,
        DSI_INST_ESCA_ULPS,
        DSI_INST_ESCA_UN1,
        DSI_INST_ESCA_UN2,
        DSI_INST_ESCA_RESET,
        DSI_INST_ESCA_UN3,
        DSI_INST_ESCA_UN4,
        DSI_INST_ESCA_UN5,
};

enum sun6i_dsi_inst_packet {
        DSI_INST_PACK_PIXEL     = 0,
        DSI_INST_PACK_COMMAND,
};

static const u32 sun6i_dsi_ecc_array[] = {
        [0] = (BIT(0) | BIT(1) | BIT(2) | BIT(4) | BIT(5) | BIT(7) | BIT(10) |
               BIT(11) | BIT(13) | BIT(16) | BIT(20) | BIT(21) | BIT(22) |
               BIT(23)),
        [1] = (BIT(0) | BIT(1) | BIT(3) | BIT(4) | BIT(6) | BIT(8) | BIT(10) |
               BIT(12) | BIT(14) | BIT(17) | BIT(20) | BIT(21) | BIT(22) |
               BIT(23)),
        [2] = (BIT(0) | BIT(2) | BIT(3) | BIT(5) | BIT(6) | BIT(9) | BIT(11) |
               BIT(12) | BIT(15) | BIT(18) | BIT(20) | BIT(21) | BIT(22)),
        [3] = (BIT(1) | BIT(2) | BIT(3) | BIT(7) | BIT(8) | BIT(9) | BIT(13) |
               BIT(14) | BIT(15) | BIT(19) | BIT(20) | BIT(21) | BIT(23)),
        [4] = (BIT(4) | BIT(5) | BIT(6) | BIT(7) | BIT(8) | BIT(9) | BIT(16) |
               BIT(17) | BIT(18) | BIT(19) | BIT(20) | BIT(22) | BIT(23)),
        [5] = (BIT(10) | BIT(11) | BIT(12) | BIT(13) | BIT(14) | BIT(15) |
               BIT(16) | BIT(17) | BIT(18) | BIT(19) | BIT(21) | BIT(22) |
               BIT(23)),
};

static u32 sun6i_dsi_ecc_compute(unsigned int data)
{
        int i;
        u8 ecc = 0;

        for (i = 0; i < ARRAY_SIZE(sun6i_dsi_ecc_array); i++) {
                u32 field = sun6i_dsi_ecc_array[i];
                bool init = false;
                u8 val = 0;
                int j;

                for (j = 0; j < 24; j++) {
                        if (!(BIT(j) & field))
                                continue;

                        if (!init) {
                                val = (BIT(j) & data) ? 1 : 0;
                                init = true;
                        } else {
                                val ^= (BIT(j) & data) ? 1 : 0;
                        }
                }

                ecc |= val << i;
        }

        return ecc;
}

static u16 const crc_ccitt_table[256] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

static inline u16 crc_ccitt_byte(u16 crc, const u8 c)
{
	return (crc >> 8) ^ crc_ccitt_table[(crc ^ c) & 0xff];
}

static u16 crc_ccitt(u16 crc, u8 const *buffer, size_t len)
{
	while (len--)
		crc = crc_ccitt_byte(crc, *buffer++);
	return crc;
}

static u16 sun6i_dsi_crc_compute(u8 const *buffer, size_t len)
{
        return crc_ccitt(0xffff, buffer, len);
}

static u16 sun6i_dsi_crc_repeat(u8 pd, u8 *buffer, size_t len)
{
        memset(buffer, pd, len);

        return sun6i_dsi_crc_compute(buffer, len);
}

static u32 sun6i_dsi_build_sync_pkt(u8 dt, u8 vc, u8 d0, u8 d1)
{
        u32 val = dt & 0x3f;

        val |= (vc & 3) << 6;
        val |= (d0 & 0xff) << 8;
        val |= (d1 & 0xff) << 16;
        val |= sun6i_dsi_ecc_compute(val) << 24;

        return val;
}

static u32 sun6i_dsi_build_blk0_pkt(u8 vc, u16 wc)
{
        return sun6i_dsi_build_sync_pkt(MIPI_DSI_BLANKING_PACKET, vc,
                                        wc & 0xff, wc >> 8);
}

static u32 sun6i_dsi_build_blk1_pkt(u16 pd, u8 *buffer, size_t len)
{
        u32 val = SUN6I_DSI_BLK_PD(pd);

        return val | SUN6I_DSI_BLK_PF(sun6i_dsi_crc_repeat(pd, buffer, len));
}

static uint32_t dsi_read(unsigned long reg)
{
	return readl(DSI_BASE + reg);
}

static void dsi_write(unsigned long reg, uint32_t val)
{
#if DUMP_DSI_INIT
	bool skip = (reg == 0x0200 && val == 0x06000200) || (reg == 0x0048 && val == 0x000f0004);
	if (!skip)
		printf("\t{ 0x%04lx, 0x%08x },\n", reg, val);
#endif
	writel(val, DSI_BASE + reg);
}

static void dsi_update_bits(unsigned long reg, uint32_t mask, uint32_t val)
{
	uint32_t tmp = readl(DSI_BASE + reg);
#if DUMP_DSI_INIT
	if (reg == 0x10 && mask == 1 && val == 1) {
		printf("\t{ MAGIC_COMMIT, 0 },\n");
	} else {
		printf("dsi_update_bits: 0x%08lx : %08x -> (%08x) %08x\n", DSI_BASE + reg, tmp, mask, val);
	}
#endif
	tmp &= ~mask;
	writel(tmp | val, DSI_BASE + reg);
}

static void sun6i_dsi_inst_setup(enum sun6i_dsi_inst_id id,
				 enum sun6i_dsi_inst_mode mode,
				 bool clock, u8 data,
				 enum sun6i_dsi_inst_packet packet,
				 enum sun6i_dsi_inst_escape escape)
{
	dsi_write(SUN6I_DSI_INST_FUNC_REG(id),
		  SUN6I_DSI_INST_FUNC_INST_MODE(mode) |
		  SUN6I_DSI_INST_FUNC_ESCAPE_ENTRY(escape) |
		  SUN6I_DSI_INST_FUNC_TRANS_PACKET(packet) |
		  (clock ? SUN6I_DSI_INST_FUNC_LANE_CEN : 0) |
		  SUN6I_DSI_INST_FUNC_LANE_DEN(data));
}

static void sun6i_dsi_inst_init(void)
{
        u8 lanes_mask = GENMASK(PANEL_LANES - 1, 0);

	sun6i_dsi_inst_setup(DSI_INST_ID_LP11, DSI_INST_MODE_STOP,
			     true, lanes_mask, 0, 0);

	sun6i_dsi_inst_setup(DSI_INST_ID_TBA, DSI_INST_MODE_TBA,
			     false, 1, 0, 0);

	sun6i_dsi_inst_setup(DSI_INST_ID_HSC, DSI_INST_MODE_HS,
			     true, 0, DSI_INST_PACK_PIXEL, 0);

	sun6i_dsi_inst_setup(DSI_INST_ID_HSD, DSI_INST_MODE_HS,
			     false, lanes_mask, DSI_INST_PACK_PIXEL, 0);

	sun6i_dsi_inst_setup(DSI_INST_ID_LPDT, DSI_INST_MODE_ESCAPE,
			     false, 1, DSI_INST_PACK_COMMAND,
			     DSI_INST_ESCA_LPDT);

	sun6i_dsi_inst_setup(DSI_INST_ID_HSCEXIT, DSI_INST_MODE_HSCEXIT,
			     true, 0, 0, 0);

	sun6i_dsi_inst_setup(DSI_INST_ID_NOP, DSI_INST_MODE_STOP,
			     false, lanes_mask, 0, 0);

	sun6i_dsi_inst_setup(DSI_INST_ID_DLY, DSI_INST_MODE_NOP,
			     true, lanes_mask, 0, 0);

	dsi_write(SUN6I_DSI_INST_JUMP_CFG_REG(0),
		  SUN6I_DSI_INST_JUMP_CFG_POINT(DSI_INST_ID_NOP) |
		  SUN6I_DSI_INST_JUMP_CFG_TO(DSI_INST_ID_HSCEXIT) |
		  SUN6I_DSI_INST_JUMP_CFG_NUM(1));
};

static u16 sun6i_dsi_get_line_num(void)
{
	unsigned int Bpp = PANEL_DSI_BPP / 8;

	return PANEL_HTOTAL * Bpp / PANEL_LANES;
}

#define SUN6I_DSI_TCON_DIV	6

static u16 sun6i_dsi_get_drq_edge0(u16 line_num, u16 edge1)
{
	u16 edge0 = edge1;

	edge0 += (PANEL_HDISPLAY + 40) * SUN6I_DSI_TCON_DIV / 8;

	if (edge0 > line_num)
		return edge0 - line_num;

	return 1;
}

static u16 sun6i_dsi_get_drq_edge1(u16 line_num)
{
	unsigned int Bpp = PANEL_DSI_BPP / 8;
	unsigned int hbp = PANEL_HTOTAL - PANEL_HSYNC_END;
	u16 edge1;

	edge1 = SUN6I_DSI_SYNC_POINT;
	edge1 += (PANEL_HDISPLAY + hbp + 20) * Bpp / PANEL_LANES;

	if (edge1 > line_num)
		return line_num;

	return edge1;
}

static void sun6i_dsi_setup_burst(void)
{
        u32 val = 0;

        if (PANEL_BURST) {
                u16 line_num = sun6i_dsi_get_line_num();
                u16 edge0, edge1;

                edge1 = sun6i_dsi_get_drq_edge1(line_num);
                edge0 = sun6i_dsi_get_drq_edge0(line_num, edge1);

                dsi_write(SUN6I_DSI_BURST_DRQ_REG,
                             SUN6I_DSI_BURST_DRQ_EDGE0(edge0) |
                             SUN6I_DSI_BURST_DRQ_EDGE1(edge1));

                dsi_write(SUN6I_DSI_BURST_LINE_REG,
                             SUN6I_DSI_BURST_LINE_NUM(line_num) |
                             SUN6I_DSI_BURST_LINE_SYNC_POINT(SUN6I_DSI_SYNC_POINT));

                val = SUN6I_DSI_TCON_DRQ_ENABLE_MODE;
        } else if ((PANEL_HSYNC_START - PANEL_HDISPLAY) > 20) {
                /* Maaaaaagic */
                u16 drq = (PANEL_HSYNC_START - PANEL_HDISPLAY) - 20;

                drq *= PANEL_DSI_BPP;
                drq /= 32;

                val = (SUN6I_DSI_TCON_DRQ_ENABLE_MODE |
                       SUN6I_DSI_TCON_DRQ_SET(drq));
        }

        dsi_write(SUN6I_DSI_TCON_DRQ_REG, val);
}

static void sun6i_dsi_setup_inst_loop(void)
{
        u16 delay = 50 - 1;

        if (PANEL_BURST) {
                u32 hsync_porch = (PANEL_HTOTAL - PANEL_HDISPLAY) * 150;

                delay = (hsync_porch / ((PANEL_CLOCK / 1000) * 8));
                delay -= 50;
        }

	dsi_write(SUN6I_DSI_INST_LOOP_SEL_REG,
		  2 << (4 * DSI_INST_ID_LP11) |
		  3 << (4 * DSI_INST_ID_DLY));

	dsi_write(SUN6I_DSI_INST_LOOP_NUM_REG(0),
		  SUN6I_DSI_INST_LOOP_NUM_N0(50 - 1) |
		  SUN6I_DSI_INST_LOOP_NUM_N1(delay));
	dsi_write(SUN6I_DSI_INST_LOOP_NUM_REG(1),
		  SUN6I_DSI_INST_LOOP_NUM_N0(50 - 1) |
		  SUN6I_DSI_INST_LOOP_NUM_N1(delay));
}

static void sun6i_dsi_setup_format(unsigned channel)
{
        u32 val = SUN6I_DSI_PIXEL_PH_VC(channel);
        u8 dt, fmt;
        u16 wc;

        /* MIPI_DSI_FMT_RGB888 */
	dt = MIPI_DSI_PACKED_PIXEL_STREAM_24;
	fmt = 8;

        val |= SUN6I_DSI_PIXEL_PH_DT(dt);

	wc = PANEL_HDISPLAY * PANEL_DSI_BPP / 8;
	val |= SUN6I_DSI_PIXEL_PH_WC(wc);
	val |= SUN6I_DSI_PIXEL_PH_ECC(sun6i_dsi_ecc_compute(val));

	dsi_write(SUN6I_DSI_PIXEL_PH_REG, val);

	dsi_write(SUN6I_DSI_PIXEL_PF0_REG,
		  SUN6I_DSI_PIXEL_PF0_CRC_FORCE(0xffff));

	dsi_write(SUN6I_DSI_PIXEL_PF1_REG,
		  SUN6I_DSI_PIXEL_PF1_CRC_INIT_LINE0(0xffff) |
		  SUN6I_DSI_PIXEL_PF1_CRC_INIT_LINEN(0xffff));

	dsi_write(SUN6I_DSI_PIXEL_CTL0_REG,
		  SUN6I_DSI_PIXEL_CTL0_PD_PLUG_DISABLE |
		  SUN6I_DSI_PIXEL_CTL0_FORMAT(fmt));
}

static void sun6i_dsi_setup_timings(unsigned channel)
{
        unsigned int Bpp = PANEL_DSI_BPP / 8;
        u16 hbp = 0, hfp = 0, hsa = 0, hblk = 0, vblk = 0;
        u32 basic_ctl = 0;
        size_t bytes;
        u8 *buffer;

        /* Do all timing calculations up front to allocate buffer space */

        if (PANEL_BURST) {
                hblk = PANEL_HDISPLAY * Bpp;
                basic_ctl = SUN6I_DSI_BASIC_CTL_VIDEO_BURST |
                            SUN6I_DSI_BASIC_CTL_HSA_HSE_DIS |
                            SUN6I_DSI_BASIC_CTL_HBP_DIS;

                if (PANEL_LANES == 4)
                        basic_ctl |= SUN6I_DSI_BASIC_CTL_TRAIL_FILL |
                                     SUN6I_DSI_BASIC_CTL_TRAIL_INV(0xc);
        } else {
                /*
                 * A sync period is composed of a blanking packet (4
                 * bytes + payload + 2 bytes) and a sync event packet
                 * (4 bytes). Its minimal size is therefore 10 bytes
                 */
#define HSA_PACKET_OVERHEAD     10
                hsa = max((unsigned int)HSA_PACKET_OVERHEAD,
                          (PANEL_HSYNC_END - PANEL_HSYNC_START) * Bpp - HSA_PACKET_OVERHEAD);

                /*
                 * The backporch is set using a blanking packet (4
                 * bytes + payload + 2 bytes). Its minimal size is
                 * therefore 6 bytes
                 */
#define HBP_PACKET_OVERHEAD     6
                hbp = max((unsigned int)HBP_PACKET_OVERHEAD,
                          (PANEL_HTOTAL - PANEL_HSYNC_END) * Bpp - HBP_PACKET_OVERHEAD);

                /*
                 * The frontporch is set using a sync event (4 bytes)
                 * and two blanking packets (each one is 4 bytes +
                 * payload + 2 bytes). Its minimal size is therefore
                 * 16 bytes
                 */
#define HFP_PACKET_OVERHEAD     16
                hfp = max((unsigned int)HFP_PACKET_OVERHEAD,
                          (PANEL_HSYNC_START - PANEL_HDISPLAY) * Bpp - HFP_PACKET_OVERHEAD);

                /*
                 * The blanking is set using a sync event (4 bytes)
                 * and a blanking packet (4 bytes + payload + 2
                 * bytes). Its minimal size is therefore 10 bytes.
                 */
#define HBLK_PACKET_OVERHEAD    10
                hblk = max((unsigned int)HBLK_PACKET_OVERHEAD,
                           (PANEL_HTOTAL - (PANEL_HSYNC_END - PANEL_HSYNC_START)) * Bpp -
                           HBLK_PACKET_OVERHEAD);

                /*
                 * And I'm not entirely sure what vblk is about. The driver in
                 * Allwinner BSP is using a rather convoluted calculation
                 * there only for 4 lanes. However, using 0 (the !4 lanes
                 * case) even with a 4 lanes screen seems to work...
                 */
                vblk = 0;
        }

        /* How many bytes do we need to send all payloads? */
        bytes = max_t(size_t, max(max(hfp, hblk), max(hsa, hbp)), vblk);
	buffer = malloc(bytes);

        dsi_write(SUN6I_DSI_BASIC_CTL_REG, basic_ctl);

	dsi_write(SUN6I_DSI_SYNC_HSS_REG,
		  sun6i_dsi_build_sync_pkt(MIPI_DSI_H_SYNC_START,
					   channel,
					   0, 0));

	dsi_write(SUN6I_DSI_SYNC_HSE_REG,
		  sun6i_dsi_build_sync_pkt(MIPI_DSI_H_SYNC_END,
					   channel,
					   0, 0));

	dsi_write(SUN6I_DSI_SYNC_VSS_REG,
		  sun6i_dsi_build_sync_pkt(MIPI_DSI_V_SYNC_START,
					   channel,
					   0, 0));

	dsi_write(SUN6I_DSI_SYNC_VSE_REG,
		  sun6i_dsi_build_sync_pkt(MIPI_DSI_V_SYNC_END,
					   channel,
					   0, 0));

	dsi_write(SUN6I_DSI_BASIC_SIZE0_REG,
		  SUN6I_DSI_BASIC_SIZE0_VSA(PANEL_VSYNC_END -
					    PANEL_VSYNC_START) |
		  SUN6I_DSI_BASIC_SIZE0_VBP(PANEL_VTOTAL -
					    PANEL_VSYNC_END));

	dsi_write(SUN6I_DSI_BASIC_SIZE1_REG,
		  SUN6I_DSI_BASIC_SIZE1_VACT(PANEL_VDISPLAY) |
		  SUN6I_DSI_BASIC_SIZE1_VT(PANEL_VTOTAL));

	/* sync */
	dsi_write(SUN6I_DSI_BLK_HSA0_REG,
		  sun6i_dsi_build_blk0_pkt(channel, hsa));
	dsi_write(SUN6I_DSI_BLK_HSA1_REG,
		  sun6i_dsi_build_blk1_pkt(0, buffer, hsa));

	/* backporch */
	dsi_write(SUN6I_DSI_BLK_HBP0_REG,
		  sun6i_dsi_build_blk0_pkt(channel, hbp));
	dsi_write(SUN6I_DSI_BLK_HBP1_REG,
		  sun6i_dsi_build_blk1_pkt(0, buffer, hbp));

	/* frontporch */
	dsi_write(SUN6I_DSI_BLK_HFP0_REG,
		  sun6i_dsi_build_blk0_pkt(channel, hfp));
	dsi_write(SUN6I_DSI_BLK_HFP1_REG,
		  sun6i_dsi_build_blk1_pkt(0, buffer, hfp));

	/* hblk */
	dsi_write(SUN6I_DSI_BLK_HBLK0_REG,
		  sun6i_dsi_build_blk0_pkt(channel, hblk));
	dsi_write(SUN6I_DSI_BLK_HBLK1_REG,
		  sun6i_dsi_build_blk1_pkt(0, buffer, hblk));

	/* vblk */
	dsi_write(SUN6I_DSI_BLK_VBLK0_REG,
		  sun6i_dsi_build_blk0_pkt(channel, vblk));
	dsi_write(SUN6I_DSI_BLK_VBLK1_REG,
		  sun6i_dsi_build_blk1_pkt(0, buffer, vblk));
}

static u16 sun6i_dsi_get_video_start_delay(void)
{
        u16 delay = PANEL_VTOTAL - (PANEL_VSYNC_START - PANEL_VDISPLAY) + 1;
        if (delay > PANEL_VTOTAL)
                delay = delay % PANEL_VTOTAL;
	if (delay < 1)
		delay = 1;

        return delay;
}

static void sun6i_dsi_inst_abort(void)
{
	dsi_update_bits(SUN6I_DSI_BASIC_CTL0_REG,
			SUN6I_DSI_BASIC_CTL0_INST_ST, 0);
}

static void sun6i_dsi_inst_commit(void)
{
	dsi_update_bits(SUN6I_DSI_BASIC_CTL0_REG,
			SUN6I_DSI_BASIC_CTL0_INST_ST,
			SUN6I_DSI_BASIC_CTL0_INST_ST);
}

static int sun6i_dsi_inst_wait_for_completion(void)
{
	ulong end_ts = timer_get_boot_us() + 5000;

        while (dsi_read(SUN6I_DSI_BASIC_CTL0_REG) & SUN6I_DSI_BASIC_CTL0_INST_ST) {
		if (end_ts < timer_get_boot_us())
			return -1;
	}

	return 0;
}

static int sun6i_dsi_start(enum sun6i_dsi_start_inst func)
{
        switch (func) {
	case DSI_START_LPTX:
		dsi_write(SUN6I_DSI_INST_JUMP_SEL_REG,
			  DSI_INST_ID_LPDT << (4 * DSI_INST_ID_LP11) |
			  DSI_INST_ID_END  << (4 * DSI_INST_ID_LPDT));
		break;
	case DSI_START_LPRX:
		dsi_write(SUN6I_DSI_INST_JUMP_SEL_REG,
			  DSI_INST_ID_LPDT << (4 * DSI_INST_ID_LP11) |
			  DSI_INST_ID_DLY  << (4 * DSI_INST_ID_LPDT) |
			  DSI_INST_ID_TBA  << (4 * DSI_INST_ID_DLY) |
			  DSI_INST_ID_END  << (4 * DSI_INST_ID_TBA));
		break;
	case DSI_START_HSC:
		dsi_write(SUN6I_DSI_INST_JUMP_SEL_REG,
			  DSI_INST_ID_HSC  << (4 * DSI_INST_ID_LP11) |
			  DSI_INST_ID_END  << (4 * DSI_INST_ID_HSC));
		break;
	case DSI_START_HSD:
		dsi_write(SUN6I_DSI_INST_JUMP_SEL_REG,
			  DSI_INST_ID_NOP  << (4 * DSI_INST_ID_LP11) |
			  DSI_INST_ID_HSD  << (4 * DSI_INST_ID_NOP) |
			  DSI_INST_ID_DLY  << (4 * DSI_INST_ID_HSD) |
			  DSI_INST_ID_NOP  << (4 * DSI_INST_ID_DLY) |
			  DSI_INST_ID_END  << (4 * DSI_INST_ID_HSCEXIT));
		break;
	default:
		dsi_write(SUN6I_DSI_INST_JUMP_SEL_REG,
			  DSI_INST_ID_END  << (4 * DSI_INST_ID_LP11));
                break;
        }

        sun6i_dsi_inst_commit();

        if (func == DSI_START_HSC)
		dsi_update_bits(SUN6I_DSI_INST_FUNC_REG(DSI_INST_ID_LP11),
				SUN6I_DSI_INST_FUNC_LANE_CEN, 0);

        return 0;
}

static u32 sun6i_dsi_dcs_build_pkt_hdr(u8 type, const u8* buf, unsigned len)
{
        u32 pkt = type;

        if (type == MIPI_DSI_DCS_LONG_WRITE) {
                pkt |= (len & 0xffff) << 8;
                pkt |= ((len >> 8) & 0xffff) << 16;
        } else {
                pkt |= buf[0] << 8;
                if (len > 1)
                        pkt |= buf[1] << 16;
        }

        pkt |= sun6i_dsi_ecc_compute(pkt) << 24;

        return pkt;
}

ssize_t mipi_dsi_dcs_write(const u8 *data, size_t len)
{
        int ret;

        dsi_write(SUN6I_DSI_CMD_CTL_REG,
                     SUN6I_DSI_CMD_CTL_RX_OVERFLOW |
                     SUN6I_DSI_CMD_CTL_RX_FLAG |
                     SUN6I_DSI_CMD_CTL_TX_FLAG);

	if (len >= 1 && len <= 2) {
		// short packet
		dsi_write(SUN6I_DSI_CMD_TX_REG(0),
			  sun6i_dsi_dcs_build_pkt_hdr(len == 1 ?
				MIPI_DSI_DCS_SHORT_WRITE : MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				data, len));

		dsi_write(SUN6I_DSI_CMD_CTL_REG, (4 - 1));

		sun6i_dsi_start(DSI_START_LPTX);
	} else if (len > 2) {
		int bounce_len = 0;
		u8 *bounce;
		u16 crc;

		dsi_write(SUN6I_DSI_CMD_TX_REG(0),
			  sun6i_dsi_dcs_build_pkt_hdr(MIPI_DSI_DCS_LONG_WRITE, data, len));

		bounce = zalloc(len + sizeof(crc) + 4);

		memcpy(bounce, data, len);
		bounce_len += len;

		crc = sun6i_dsi_crc_compute(bounce, len);
		memcpy(bounce + bounce_len, &crc, sizeof(crc));
		bounce_len += sizeof(crc);

		for (unsigned i = 0; i < DIV_ROUND_UP(bounce_len, 4); i++)
			dsi_write(SUN6I_DSI_CMD_TX_REG(1 + i), ((u32*)bounce)[i]);

		dsi_write(SUN6I_DSI_CMD_CTL_REG, bounce_len + 4 - 1);

		sun6i_dsi_start(DSI_START_LPTX);
		/*
		 * TODO: There's some bits (reg 0x200, bits 8/9) that
		 * apparently can be used to check whether the data have been
		 * sent, but I couldn't get it to work reliably.
		 */
	} else {
		return -1;
	}

	ret = sun6i_dsi_inst_wait_for_completion();
	if (ret < 0) {
		sun6i_dsi_inst_abort();
		return ret;
	}

	return 0;
}

static void dsi_init(void)
{
	display_board_init();

	/* mipi dsi bus enable */
	setbits_le32(CCU_BUS_CLK_GATE0, 1 << 1);
	setbits_le32(CCU_BUS_SOFT_RST0, 1 << 1);

        /*
         * Enable the DSI block.
         */
#if DUMP_DSI_INIT
	printf("struct reg_inst dsi_init_seq[] = {\n");
#endif
        dsi_write(SUN6I_DSI_CTL_REG, SUN6I_DSI_CTL_EN);

        dsi_write(SUN6I_DSI_BASIC_CTL0_REG,
		  SUN6I_DSI_BASIC_CTL0_ECC_EN | SUN6I_DSI_BASIC_CTL0_CRC_EN);

        dsi_write(SUN6I_DSI_TRANS_START_REG, 10);
        dsi_write(SUN6I_DSI_TRANS_ZERO_REG, 0);

        sun6i_dsi_inst_init();

        dsi_write(SUN6I_DSI_DEBUG_DATA_REG, 0xff);

	u16 delay = sun6i_dsi_get_video_start_delay();
	dsi_write(SUN6I_DSI_BASIC_CTL1_REG,
		  SUN6I_DSI_BASIC_CTL1_VIDEO_ST_DELAY(delay) |
		  SUN6I_DSI_BASIC_CTL1_VIDEO_FILL |
		  SUN6I_DSI_BASIC_CTL1_VIDEO_PRECISION |
		  SUN6I_DSI_BASIC_CTL1_VIDEO_MODE);

        sun6i_dsi_setup_burst();
        sun6i_dsi_setup_inst_loop();
        sun6i_dsi_setup_format(0);
        sun6i_dsi_setup_timings(0);

#if DUMP_DSI_INIT
	printf("};\n");
#endif

	dphy_enable();

	// deassert reset
	gpio_direction_output(SUNXI_GPD(23), 1); // PD23 - LCD-RST (active low)

	// wait for initialization (5-120ms, depending on mode... hmm?)
	udelay(15000);

#if DUMP_DSI_INIT
	printf("\nstruct reg_inst dsi_panel_init_seq[] = {\n");
#endif
	panel_init();
#if DUMP_DSI_INIT
	printf("};\n");
#endif

        sun6i_dsi_start(DSI_START_HSC);

        udelay(1000);

        sun6i_dsi_start(DSI_START_HSD);
}

// }}}
// {{{ DSI SMALL

#define MAGIC_COMMIT 0xffffu
#define MAGIC_SLEEP 0xfffeu

struct reg_inst {
	u16 inst;
	u32 val;
} __packed;

struct reg_inst dsi_init_seq[] = {
	{ 0x0000, 0x00000001 },
	{ 0x0010, 0x00030000 },
	{ 0x0060, 0x0000000a },
	{ 0x0078, 0x00000000 },
	{ 0x0020, 0x0000001f },
	{ 0x0024, 0x10000001 },
	{ 0x0028, 0x20000010 },
	{ 0x002c, 0x2000000f },
	{ 0x0030, 0x30100001 },
	{ 0x0034, 0x40000010 },
	{ 0x0038, 0x0000000f },
	{ 0x003c, 0x5000001f },
	{ 0x004c, 0x00560001 },
	{ 0x02f8, 0x000000ff },
	{ 0x0014, 0x00005bc7 },
	{ 0x007c, 0x10000007 },
	{ 0x0040, 0x30000002 },
	{ 0x0044, 0x00310031 },
	{ 0x0054, 0x00310031 },
	{ 0x0090, 0x1308703e },
	{ 0x0098, 0x0000ffff },
	{ 0x009c, 0xffffffff },
	{ 0x0080, 0x00010008 },
	{ 0x000c, 0x00000000 },
	{ 0x00b0, 0x12000021 },
	{ 0x00b4, 0x01000031 },
	{ 0x00b8, 0x07000001 },
	{ 0x00bc, 0x14000011 },
	{ 0x0018, 0x0011000a },
	{ 0x001c, 0x05cd05a0 },
	{ 0x00c0, 0x09004a19 },
	{ 0x00c4, 0x50b40000 },
	{ 0x00c8, 0x35005419 },
	{ 0x00cc, 0x757a0000 },
	{ 0x00d0, 0x09004a19 },
	{ 0x00d4, 0x50b40000 },
	{ 0x00e0, 0x0c091a19 },
	{ 0x00e4, 0x72bd0000 },
	{ 0x00e8, 0x1a000019 },
	{ 0x00ec, 0xffff0000 },
};

struct reg_inst dsi_panel_init_seq[] = {
	{ 0x0300, 0x2c000439 },
	{ 0x0304, 0x8312f1b9 },
	{ 0x0308, 0x00005d84 },
	{ 0x0200, 0x00000009 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x2f001c39 },
	{ 0x0304, 0x058133ba },
	{ 0x0308, 0x200e0ef9 },
	{ 0x030c, 0x00000000 },
	{ 0x0310, 0x44000000 },
	{ 0x0314, 0x0a910025 },
	{ 0x0318, 0x4f020000 },
	{ 0x031c, 0x37000011 },
	{ 0x0320, 0x0000e22c },
	{ 0x0200, 0x00000021 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x36000539 },
	{ 0x0304, 0x202225b8 },
	{ 0x0308, 0x00720303 },
	{ 0x0200, 0x0000000a },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x2c000b39 },
	{ 0x0304, 0x051010b3 },
	{ 0x0308, 0x00ff0305 },
	{ 0x030c, 0x6f000000 },
	{ 0x0310, 0x000000bc },
	{ 0x0200, 0x00000010 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x36000a39 },
	{ 0x0304, 0x507373c0 },
	{ 0x0308, 0x08c00050 },
	{ 0x030c, 0x6a1b0070 },
	{ 0x0200, 0x0000000f },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x354ebc15 },
	{ 0x0200, 0x00000003 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x220bcc15 },
	{ 0x0200, 0x00000003 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x2280b415 },
	{ 0x0200, 0x00000003 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x2c000439 },
	{ 0x0304, 0xf012f0b2 },
	{ 0x0308, 0x00008651 },
	{ 0x0200, 0x00000009 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x0f000f39 },
	{ 0x0304, 0x0b0000e3 },
	{ 0x0308, 0x0010100b },
	{ 0x030c, 0xff000000 },
	{ 0x0310, 0x3610c000 },
	{ 0x0314, 0x0000000f },
	{ 0x0200, 0x00000014 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x30000639 },
	{ 0x0304, 0xff0001c6 },
	{ 0x0308, 0x258e00ff },
	{ 0x0200, 0x0000000b },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x13000d39 },
	{ 0x0304, 0x320074c1 },
	{ 0x0308, 0xfff17732 },
	{ 0x030c, 0x77ccccff },
	{ 0x0310, 0x00e46977 },
	{ 0x0200, 0x00000012 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x09000339 },
	{ 0x0304, 0x7b0707b5 },
	{ 0x0308, 0x000000b3 },
	{ 0x0200, 0x00000008 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x09000339 },
	{ 0x0304, 0x552c2cb6 },
	{ 0x0308, 0x00000004 },
	{ 0x0200, 0x00000008 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x2c000439 },
	{ 0x0304, 0x001102bf },
	{ 0x0308, 0x0000e9b5 },
	{ 0x0200, 0x00000009 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x25004039 },
	{ 0x0304, 0x061082e9 },
	{ 0x0308, 0xa50aa205 },
	{ 0x030c, 0x37233112 },
	{ 0x0310, 0x27bc0483 },
	{ 0x0314, 0x03000c38 },
	{ 0x0318, 0x0c000000 },
	{ 0x031c, 0x00000300 },
	{ 0x0320, 0x31757500 },
	{ 0x0324, 0x88888888 },
	{ 0x0328, 0x88138888 },
	{ 0x032c, 0x88206464 },
	{ 0x0330, 0x88888888 },
	{ 0x0334, 0x00880288 },
	{ 0x0338, 0x00000000 },
	{ 0x033c, 0x00000000 },
	{ 0x0340, 0x00000000 },
	{ 0x0344, 0x00000365 },
	{ 0x0200, 0x00000045 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x1a003e39 },
	{ 0x0304, 0x002102ea },
	{ 0x0308, 0x00000000 },
	{ 0x030c, 0x00000000 },
	{ 0x0310, 0x02460200 },
	{ 0x0314, 0x88888888 },
	{ 0x0318, 0x88648888 },
	{ 0x031c, 0x88135713 },
	{ 0x0320, 0x88888888 },
	{ 0x0324, 0x23887588 },
	{ 0x0328, 0x02000014 },
	{ 0x032c, 0x00000000 },
	{ 0x0330, 0x00000000 },
	{ 0x0334, 0x00000000 },
	{ 0x0338, 0x03000000 },
	{ 0x033c, 0x0000a50a },
	{ 0x0340, 0x1b240000 },
	{ 0x0200, 0x00000043 },
	{ MAGIC_COMMIT, 0 },
	{ 0x0300, 0x20002339 },
	{ 0x0304, 0x0d0900e0 },
	{ 0x0308, 0x413c2723 },
	{ 0x030c, 0x0e0d0735 },
	{ 0x0310, 0x12101312 },
	{ 0x0314, 0x09001812 },
	{ 0x0318, 0x3c27230d },
	{ 0x031c, 0x0d073541 },
	{ 0x0320, 0x1013120e },
	{ 0x0324, 0x93181212 },
	{ 0x0328, 0x000000bf },
	{ 0x0200, 0x00000028 },
	{ MAGIC_COMMIT, 0 },

	// sleep out
	{ 0x0300, 0x36001105 },
	{ 0x0200, 0x00000003 },
	{ MAGIC_COMMIT, 0 },
	{ MAGIC_SLEEP, 120000 },

	// display on
	{ 0x0300, 0x1c002905 },
	{ 0x0200, 0x00000003 },
	{ MAGIC_COMMIT, 0 },
};

static int dsi_run_init_seq(struct reg_inst* insts, unsigned len)
{
	dsi_write(SUN6I_DSI_CMD_CTL_REG,
		  SUN6I_DSI_CMD_CTL_RX_OVERFLOW |
		  SUN6I_DSI_CMD_CTL_RX_FLAG |
		  SUN6I_DSI_CMD_CTL_TX_FLAG);

	for (int i = 0; i < len; i++) {
		struct reg_inst* in = &insts[i];

		if (in->inst == MAGIC_SLEEP) {
			udelay(in->val);
		} else if (in->inst == MAGIC_COMMIT) {
			sun6i_dsi_start(DSI_START_LPTX);
			int ret = sun6i_dsi_inst_wait_for_completion();
			if (ret < 0) {
				sun6i_dsi_inst_abort();
				return -1;
			}

			dsi_write(SUN6I_DSI_CMD_CTL_REG,
				  SUN6I_DSI_CMD_CTL_RX_OVERFLOW |
				  SUN6I_DSI_CMD_CTL_RX_FLAG |
				  SUN6I_DSI_CMD_CTL_TX_FLAG);
		} else {
			dsi_write(in->inst, in->val);
		}
	}

	return 0;
}

static int dsi_init_fast(void)
{
	/* mipi dsi bus enable */
	setbits_le32(CCU_BUS_CLK_GATE0, 1 << 1);
	setbits_le32(CCU_BUS_SOFT_RST0, 1 << 1);

	dsi_run_init_seq(dsi_init_seq, ARRAY_SIZE(dsi_init_seq));
	display_board_init();

	dphy_enable();

	// deassert reset
	gpio_direction_output(SUNXI_GPD(23), 1); // PD23 - LCD-RST (active low)

	// wait for initialization (5-120ms, depending on mode... hmm?)
	udelay(15000);

	dsi_run_init_seq(dsi_panel_init_seq, ARRAY_SIZE(dsi_panel_init_seq));

        sun6i_dsi_start(DSI_START_HSC);

        udelay(1000);

        sun6i_dsi_start(DSI_START_HSD);

	return 0;
}

// }}}
// {{{ TCON0

static void tcon0_init(void)
{
	struct sunxi_ccm_reg * const ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_lcdc_reg * const lcdc = (struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;
	unsigned dclk_div = 6;

	// PLL_VIDEO0 - range 192MHz - 600MHz
	//   rate = 24000000 * n / m
	//   rate = (24 * 99 / 8) = 297
	writel(CCM_PLL3_CTRL_EN | CCM_PLL3_CTRL_INTEGER_MODE |
	       CCM_PLL3_CTRL_N(99) | CCM_PLL3_CTRL_M(8),
	       &ccm->pll3_cfg);

	// PLL_MIPI
	//   rate = PLL_VIDEO0 * K * N / M    M <= 3 * N    K >= 2
	//   rate = dclk_div * PANEL_CLOCK
	//   rate = 297 * 16 / 11 = 432MHz
	//   dclk = 72 MHz
	writel(CCM_MIPI_PLL_CTRL_LDO_EN, &ccm->mipi_pll_cfg);
	udelay(100);
	writel(CCM_MIPI_PLL_CTRL_N(8) | CCM_MIPI_PLL_CTRL_K(2) |
	       CCM_MIPI_PLL_CTRL_M(11) | CCM_MIPI_PLL_CTRL_EN |
	       CCM_MIPI_PLL_CTRL_LDO_EN,
	       &ccm->mipi_pll_cfg);

	// TCON0 source MIPI_PLL
	writel(CCM_LCD_CH0_CTRL_GATE | CCM_LCD_CH0_CTRL_MIPI_PLL,
	       &ccm->lcd0_clk_cfg);

	/* Clock on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_LCD0);
	/* Reset off */
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_LCD0);

	/* Init lcdc */
	writel(0, &lcdc->ctrl); /* Disable tcon */
	writel_relaxed(0, &lcdc->int0); /* Disable all interrupts */
	writel_relaxed(0, &lcdc->int1);

	/* Disable tcon0 dot clock */
	//clrbits_le32(&lcdc->tcon0_dclk, SUNXI_LCDC_TCON0_DCLK_ENABLE);

	/* Set all io lines to tristate */
	writel_relaxed(0xffffffff, &lcdc->tcon0_io_tristate);
	writel_relaxed(0xffffffff, &lcdc->tcon1_io_tristate);

        /* mode set */

	// DCLK = MIPI_PLL / 6
	writel_relaxed(SUNXI_LCDC_TCON0_DCLK_ENABLE_1 |
			SUNXI_LCDC_TCON0_DCLK_DIV(dclk_div), &lcdc->tcon0_dclk);

	writel_relaxed(SUNXI_LCDC_TCON0_CTRL_ENABLE |
			SUNXI_LCDC_TCON0_CTRL_IF_8080, &lcdc->tcon0_ctrl);

	writel_relaxed(SUNXI_LCDC_X(PANEL_HDISPLAY) |
			SUNXI_LCDC_Y(PANEL_VDISPLAY), &lcdc->tcon0_timing_active);

#if 0
	for (int i = 0; i < 6; i++)
		writel_relaxed(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[i]);
	writel_relaxed(SUNXI_LCDC_TCON0_FRM_TAB0, &lcdc->tcon0_frm_table[0]);
	writel_relaxed(SUNXI_LCDC_TCON0_FRM_TAB1, &lcdc->tcon0_frm_table[1]);
	writel_relaxed(SUNXI_LCDC_TCON0_FRM_TAB2, &lcdc->tcon0_frm_table[2]);
	writel_relaxed(SUNXI_LCDC_TCON0_FRM_TAB3, &lcdc->tcon0_frm_table[3]);

	// not needed for PP panel
        /* Set dithering */
                //val |= SUN4I_TCON0_FRM_CTL_MODE_R;
                //val |= SUN4I_TCON0_FRM_CTL_MODE_B;
	/* Write dithering settings */
	//writel(SUN4I_TCON0_FRM_CTL_EN, &lcdc->tcon0_frm_ctrl);
#endif

        writel_relaxed(SUN4I_TCON_ECC_FIFO_EN, &lcdc->ecc_fifo);
	writel_relaxed(SUN4I_TCON0_CPU_IF_MODE_DSI |
	       SUN4I_TCON0_CPU_IF_TRI_FIFO_FLUSH |
	       SUN4I_TCON0_CPU_IF_TRI_FIFO_EN |
	       SUN4I_TCON0_CPU_IF_TRI_EN, &lcdc->tcon0_cpu_intf);

        /*
         * This looks suspicious, but it works...
         *
         * The datasheet says that this should be set higher than 20 *
         * pixel cycle, but it's not clear what a pixel cycle is.
         */
	unsigned bpp = PANEL_DSI_BPP; // RGB888
        unsigned block_space = PANEL_HTOTAL * bpp / (dclk_div * PANEL_LANES);
        block_space -= PANEL_HDISPLAY + 40;

	writel_relaxed(SUN4I_TCON0_CPU_TRI0_BLOCK_SPACE(block_space) |
	       SUN4I_TCON0_CPU_TRI0_BLOCK_SIZE(PANEL_HDISPLAY),
	       SUNXI_LCD0_BASE + SUN4I_TCON0_CPU_TRI0_REG);

        writel_relaxed(SUN4I_TCON0_CPU_TRI1_BLOCK_NUM(PANEL_VDISPLAY),
	       SUNXI_LCD0_BASE + SUN4I_TCON0_CPU_TRI1_REG);

        unsigned start_delay = (PANEL_VTOTAL - PANEL_VDISPLAY - 10 - 1);
        start_delay = start_delay * PANEL_HTOTAL * 149;
        start_delay = start_delay / (PANEL_CLOCK / 1000) / 8;
	writel_relaxed(SUN4I_TCON0_CPU_TRI2_TRANS_START_SET(10) |
	       SUN4I_TCON0_CPU_TRI2_START_DELAY(start_delay),
	       SUNXI_LCD0_BASE + SUN4I_TCON0_CPU_TRI2_REG);

        /*
         * The Allwinner BSP has a comment that the period should be
         * the display clock * 15, but uses an hardcoded 3000...
         */
        writel_relaxed(SUN4I_TCON_SAFE_PERIOD_NUM(3000) |
                     SUN4I_TCON_SAFE_PERIOD_MODE(3),
		     SUNXI_LCD0_BASE + SUN4I_TCON_SAFE_PERIOD_REG);

        /* Enable the output on the pins */
	writel(0xe0000000, &lcdc->tcon0_io_tristate);

        // enable tcon as a whole
	setbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_TCON_ENABLE);
}

// }}}
// {{{ Scaler

#define DE2_UI_SCALER_UNIT_SIZE 0x10000

/* this two macros assumes 16 fractional bits which is standard in DRM */
#define SUN8I_UI_SCALER_SCALE_MIN		1
#define SUN8I_UI_SCALER_SCALE_MAX		((1UL << 20) - 1)

#define SUN8I_UI_SCALER_SCALE_FRAC		20
#define SUN8I_UI_SCALER_PHASE_FRAC		20
#define SUN8I_UI_SCALER_COEFF_COUNT		16

#define SUN8I_SCALER_GSU_CTRL(base)		((base) + 0x0)
#define SUN8I_SCALER_GSU_OUTSIZE(base)		((base) + 0x40)
#define SUN8I_SCALER_GSU_INSIZE(base)		((base) + 0x80)
#define SUN8I_SCALER_GSU_HSTEP(base)		((base) + 0x88)
#define SUN8I_SCALER_GSU_VSTEP(base)		((base) + 0x8c)
#define SUN8I_SCALER_GSU_HPHASE(base)		((base) + 0x90)
#define SUN8I_SCALER_GSU_VPHASE(base)		((base) + 0x98)
#define SUN8I_SCALER_GSU_HCOEFF(base, index)	((base) + 0x200 + 0x4 * (index))

#define SUN8I_SCALER_GSU_CTRL_EN		BIT(0)
#define SUN8I_SCALER_GSU_CTRL_COEFF_RDY		BIT(4)

#define DE2_VI_SCALER_UNIT_BASE 0x20000
#define DE2_VI_SCALER_UNIT_SIZE 0x20000

static const u32 lan2coefftab16[] = {
	0x00004000, 0x00033ffe, 0x00063efc, 0x000a3bfb,
	0xff0f37fb, 0xfe1433fb, 0xfd192ffb, 0xfd1f29fb,
	0xfc2424fc, 0xfb291ffd, 0xfb2f19fd, 0xfb3314fe,
	0xfb370fff, 0xfb3b0a00, 0xfc3e0600, 0xfe3f0300,

	/*
	0xff053804, 0xff083801, 0xff0a3700, 0xff0e34ff,
	0xff1232fd, 0xfe162ffd, 0xfd1b2cfc, 0xfd1f28fc,
	0xfd2323fd, 0xfc281ffd, 0xfc2c1bfd, 0xfd2f16fe,
	0xfd3212ff, 0xff340eff, 0x00360a00, 0x02370700,

	0xff083207, 0xff0a3205, 0xff0d3103, 0xfe113001,
	0xfe142e00, 0xfe182bff, 0xfe1b29fe, 0xfe1f25fe,
	0xfe2222fe, 0xfe251ffe, 0xfe291bfe, 0xff2b18fe,
	0x002e14fe, 0x013010ff, 0x03310dff, 0x05310a00,

	0xff0a2e09, 0xff0c2e07, 0xff0f2d05, 0xff122c03,
	0xfe152b02, 0xfe182901, 0xfe1b2700, 0xff1e24ff,
	0xff2121ff, 0xff241eff, 0x00261bff, 0x012818ff,
	0x022a15ff, 0x032c12ff, 0x052d0fff, 0x072d0c00,

	0xff0c2a0b, 0xff0e2a09, 0xff102a07, 0xff132905,
	0xff162803, 0xff182702, 0xff1b2501, 0xff1e2300,
	0x00202000, 0x01221d00, 0x01251bff, 0x032618ff,
	0x042815ff, 0x052913ff, 0x072a10ff, 0x092a0d00,

	0xff0d280c, 0xff0f280a, 0xff112808, 0xff142706,
	0xff162605, 0xff192503, 0x001b2302, 0x001d2201,
	0x011f1f01, 0x01221d00, 0x02231b00, 0x04241800,
	0x052616ff, 0x072713ff, 0x08271100, 0x0a280e00,

	0xff0e260d, 0xff10260b, 0xff122609, 0xff142508,
	0x00152506, 0x00182305, 0x001b2203, 0x011d2002,
	0x011f1f01, 0x02201d01, 0x03221b00, 0x04231801,
	0x06241600, 0x08251300, 0x09261100, 0x0b260f00,

	0xff0e250e, 0xff10250c, 0x0011250a, 0x00142408,
	0x00162307, 0x00182206, 0x011a2104, 0x011c2003,
	0x021e1e02, 0x03201c01, 0x04211a01, 0x05221801,
	0x07231600, 0x08241400, 0x0a241200, 0x0c241000,

	0x000e240e, 0x0010240c, 0x0013230a, 0x00142309,
	0x00162208, 0x01182106, 0x011a2005, 0x021b1f04,
	0x031d1d03, 0x041e1c02, 0x05201a01, 0x06211801,
	0x07221601, 0x09231400, 0x0a231300, 0x0c231100,

	0x000f220f, 0x0011220d, 0x0013220b, 0x0015210a,
	0x01162108, 0x01182007, 0x02191f06, 0x031a1e05,
	0x041c1c04, 0x051d1b03, 0x061f1902, 0x07201801,
	0x08211601, 0x0a211500, 0x0b221300, 0x0d221100,

	0x0010210f, 0x0011210e, 0x0013210c, 0x0114200b,
	0x01161f0a, 0x02171f08, 0x03181e07, 0x031a1d06,
	0x041c1c04, 0x051d1a04, 0x071d1903, 0x081e1802,
	0x091f1602, 0x0b1f1501, 0x0c211300, 0x0e201200,

	0x00102010, 0x0012200e, 0x0013200d, 0x01151f0b,
	0x01161f0a, 0x02171e09, 0x03191d07, 0x041a1c06,
	0x051b1b05, 0x061c1a04, 0x071d1903, 0x081e1703,
	0x0a1f1601, 0x0b1f1501, 0x0d201300, 0x0e201200,

	0x00102010, 0x00121f0f, 0x00141f0d, 0x01141f0c,
	0x02161e0a, 0x03171d09, 0x03181d08, 0x041a1c06,
	0x051b1b05, 0x061c1a04, 0x081c1903, 0x091d1703,
	0x0a1e1602, 0x0c1e1501, 0x0d1f1400, 0x0e1f1201,

	0x00111e11, 0x00131e0f, 0x01131e0e, 0x02151d0c,
	0x02161d0b, 0x03171c0a, 0x04181b09, 0x05191b07,
	0x061a1a06, 0x071b1905, 0x091b1804, 0x0a1c1703,
	0x0b1d1602, 0x0c1d1502, 0x0e1d1401, 0x0f1e1300,

	0x00111e11, 0x00131d10, 0x01141d0e, 0x02151c0d,
	0x03161c0b, 0x04171b0a, 0x05171b09, 0x06181a08,
	0x07191907, 0x081a1806, 0x091a1805, 0x0a1b1704,
	0x0b1c1603, 0x0d1c1502, 0x0e1d1401, 0x0f1d1301,
	*/
};

static int sun8i_ui_scaler_coef_index(unsigned int step)
{
	unsigned int scale, int_part, float_part;
	return 0;

	scale = step >> (SUN8I_UI_SCALER_SCALE_FRAC - 3);
	int_part = scale >> 3;
	float_part = scale & 0x7;

	switch (int_part) {
	case 0:
		return 0;
	case 1:
		return float_part;
	case 2:
		return 8 + (float_part >> 1);
	case 3:
		return 12;
	case 4:
		return 13;
	default:
		return 14;
	}
}

static void mixer_write(uint32_t off, uint32_t v)
{
	uint32_t base = SUNXI_DE2_MUX0_BASE;

	writel_relaxed(v, (ulong)base + off);
}

static void sun8i_ui_scaler_setup(int layer, u32 insize, u32 outsize,
				  u32 hscale, u32 vscale)
{
	u32 base, hphase = 0, vphase = 0;
	int i, offset;

	base = DE2_VI_SCALER_UNIT_BASE +
		DE2_VI_SCALER_UNIT_SIZE * 1 +
		DE2_UI_SCALER_UNIT_SIZE * layer;

	hphase <<= SUN8I_UI_SCALER_PHASE_FRAC - 16;
	vphase <<= SUN8I_UI_SCALER_PHASE_FRAC - 16;
	//hscale <<= SUN8I_UI_SCALER_SCALE_FRAC - 16;
	//vscale <<= SUN8I_UI_SCALER_SCALE_FRAC - 16;

	mixer_write(SUN8I_SCALER_GSU_OUTSIZE(base), outsize);
	mixer_write(SUN8I_SCALER_GSU_INSIZE(base), insize);
	mixer_write(SUN8I_SCALER_GSU_HSTEP(base), hscale);
	mixer_write(SUN8I_SCALER_GSU_VSTEP(base), vscale);
	mixer_write(SUN8I_SCALER_GSU_HPHASE(base), hphase);
	mixer_write(SUN8I_SCALER_GSU_VPHASE(base), vphase);
	offset = sun8i_ui_scaler_coef_index(hscale) *
			SUN8I_UI_SCALER_COEFF_COUNT;
	for (i = 0; i < SUN8I_UI_SCALER_COEFF_COUNT; i++)
		mixer_write(SUN8I_SCALER_GSU_HCOEFF(base, i),
			     lan2coefftab16[offset + i]);

	mixer_write(SUN8I_SCALER_GSU_CTRL(base),
		    SUN8I_SCALER_GSU_CTRL_EN |
		    SUN8I_SCALER_GSU_CTRL_COEFF_RDY);
}

static void sun8i_ui_scaler_disable(int layer)
{
	u32 base;

	base = DE2_VI_SCALER_UNIT_BASE +
		DE2_VI_SCALER_UNIT_SIZE * 1 +
		DE2_UI_SCALER_UNIT_SIZE * layer;

	mixer_write(SUN8I_SCALER_GSU_CTRL(base), 0);
}

// }}}
// {{{ DE2

static void de2_init(void)
{
	ulong de_mux_base = SUNXI_DE2_MUX0_BASE;
	u32 reg_value;

	struct sunxi_ccm_reg * const ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct de_clk * const de_clk_regs = (struct de_clk *)(SUNXI_DE2_BASE);
	struct de_glb * const de_glb_regs = (struct de_glb *)(de_mux_base + SUNXI_DE2_MUX_GLB_REGS);
	//struct de_bld * const de_bld_regs = (struct de_bld *)(de_mux_base + SUNXI_DE2_MUX_BLD_REGS);
	//struct de_ui * const de_ui_regs = (struct de_ui *)(de_mux_base + SUNXI_DE2_MUX_CHAN_REGS + SUNXI_DE2_MUX_CHAN_SZ * 1);
	struct de_csc * const de_csc_regs = (struct de_csc *)(de_mux_base + SUNXI_DE2_MUX_DCSC_REGS);

	/* Set SRAM for video use */
	reg_value = readl(SUNXI_SRAMC_BASE + 0x04);
	reg_value &= ~(1u << 24);
	writel(reg_value, SUNXI_SRAMC_BASE + 0x04);

	/* Setup DE2 PLL */
	clock_set_pll_de(297000000);
	//clock_set_pll_de(432000000);

	/* Enable DE2 special clock */
	clrsetbits_le32(&ccm->de_clk_cfg,
			CCM_DE2_CTRL_PLL_MASK,
			CCM_DE2_CTRL_PLL10 | CCM_DE_CTRL_GATE);

	/* Enable DE2 ahb */
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_DE);
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_DE);

	/* Enable clock for mixer 0, set route MIXER0->TCON0 */
	setbits_le32(&de_clk_regs->gate_cfg, BIT(0));
	setbits_le32(&de_clk_regs->rst_cfg, BIT(0));
	setbits_le32(&de_clk_regs->bus_cfg, BIT(0));
	clrbits_le32(&de_clk_regs->sel_cfg, 1);

	/* Clear all registers */
	for (unsigned i = 0; i < 0x6000; i += 4)
		writel_relaxed(0, de_mux_base + i);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_VSU_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_GSU1_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_GSU2_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_GSU3_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_FCE_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_BWS_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_LTI_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_PEAK_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_ASE_REGS);
	writel_relaxed(0, de_mux_base + SUNXI_DE2_MUX_FCC_REGS);
	writel_relaxed(0, &de_csc_regs->csc_ctl);

	/* Enable mixer */
	writel(1, &de_glb_regs->ctl);
}

// }}}
// {{{ Board resources

void backlight_enable(uint32_t pct)
{
	struct sunxi_pwm *pwm = (struct sunxi_pwm *)0x01f03800;

	// 1.0 has incorrectly documented non-presence of PH10, the
	// circuit is in fact the same as on 1.1+
//	gpio_direction_output(SUNXI_GPL(10), 0); // enable backlight

	sunxi_gpio_set_cfgpin(SUNXI_GPL(10), SUN50I_GPL_R_PWM);

        clrbits_le32(&pwm->ctrl, SUNXI_PWM_CTRL_CLK_GATE);
        writel(SUNXI_PWM_CH0_PERIOD_PRD(1199) |
               SUNXI_PWM_CH0_PERIOD_DUTY(1199 * pct / 100), &pwm->ch0_period);
        writel(0xf | SUNXI_PWM_CTRL_CLK_GATE | SUNXI_PWM_CTRL_ENABLE0, &pwm->ctrl);

	gpio_direction_output(SUNXI_GPH(10), 1); // enable backlight
}

void display_board_init(void)
{
        /*
	 * This needs to handle power on and SoC reset (power supplies
	 * already on):
	 *
	 * Power on:
	 * - RESX needs to be held low for 10ms after applying power
	 * - We can start sending commands 5ms after deasserting reset
	 * - We can enable display (DISPON)  after 120ms after sleep out
	 * SoC reset:
	 * - Display resets after > 10us pulse low on RESX line.
	 */

	// assert reset
	gpio_direction_output(SUNXI_GPD(23), 0); // PD23 - LCD-RST (active low)

	// dldo1 3.3V
	pmic_write(0x15, 0x1a);
	pmic_setbits(0x12, BIT(3));

	// ldo_io0 3.3V
	pmic_write(0x91, 0x1a); // set LDO voltage to 3.3V
	pmic_write(0x90, 0x03); // enable LDO mode on GPIO0

	// dldo2 1.8V
	pmic_write(0x16, 0x0b);
	//pmic_write(0x16, 0x1a);
	pmic_setbits(0x12, BIT(4));

	// wait for power supplies and power-on init
	udelay(15000);
}

// }}}
// {{{ Display init

// this initializes DE2 + TCON + DSI + PANEL + BACKLIGHT and creates a framebuffer
bool display_init(void)
{
	tcon0_init();

#if DSI_FULL_INIT
	dsi_init();
#else
	dsi_init_fast();
#endif
	de2_init();

	//dump_dsi_registers();
	//dump_de2_registers();

	return true;
}

// }}}
// {{{ Compositor

bool display_frame_done(void)
{
	struct sunxi_lcdc_reg * const lcdc = (struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;

	uint32_t status = readl(&lcdc->int0);
	if (status & BIT(11)) {
		writel(0, &lcdc->int0); /* Disable all interrupts */
	}

	return status & BIT(11);
}

void display_commit(struct display* d)
{
	ulong de_mux_base = SUNXI_DE2_MUX0_BASE;
	struct de_glb * const de_glb_regs = (struct de_glb *)(de_mux_base + SUNXI_DE2_MUX_GLB_REGS);
	struct de_bld * const de_bld_regs = (struct de_bld *)(de_mux_base + SUNXI_DE2_MUX_BLD_REGS);
	u32 disp_size = SUNXI_DE2_WH(PANEL_HDISPLAY, PANEL_VDISPLAY);

//	while (readl(&de_glb_regs->status) & 0x3 == 0);
//	writel(0, &de_glb_regs->status);

	//writel(disp_size, &de_glb_regs->size);

	/* Configure blender */
	writel_relaxed(0xff000000, &de_bld_regs->bkcolor);
	writel_relaxed(0, &de_bld_regs->premultiply);
	//writel_relaxed(disp_size, &de_bld_regs->output_size);

	uint32_t route_reg = 0, fcolor_ctl = 0;
	int pipe = 0;
	for (int i = 0; i < ARRAY_SIZE(d->planes); i++) {
		struct display_plane* p = &d->planes[i];
		struct de_ui * const de_ui_regs = (struct de_ui *)(de_mux_base + SUNXI_DE2_MUX_CHAN_REGS + SUNXI_DE2_MUX_CHAN_SZ * (1 + i));
		u32 format = SUNXI_DE2_UI_CFG_ATTR_FMT(SUNXI_DE2_FORMAT_ARGB_8888);
		if (i == 0)
			format = SUNXI_DE2_UI_CFG_ATTR_FMT(SUNXI_DE2_FORMAT_XRGB_8888);

		/*
		 * We need to clamp output to the display panel area.
		 *
		 * If dst_w/h == src_w/h we just clip the dst and src
		 * areas in the same way.
		 *
		 * If dst_w/h != src_w/h, that means that output is scaled.
		 * We need to clip the dst area to the display size, but src
		 * area needs to be cliped according to the src/dst scale.
		 */
		u32 dst_ox = -min_t(int, p->dst_x, 0);
		u32 dst_oy = -min_t(int, p->dst_y, 0);
		u32 dst_x = clamp(p->dst_x, 0, PANEL_WIDTH);
		u32 dst_y = clamp(p->dst_y, 0, PANEL_HEIGHT);
		u32 dst_w = clamp((int)p->dst_w - (int)dst_ox, 0, (int)PANEL_WIDTH - (int)dst_x);
		u32 dst_h = clamp((int)p->dst_h - (int)dst_oy, 0, (int)PANEL_HEIGHT - (int)dst_y);

		u32 src_x = p->src_x + dst_ox * p->src_w / p->dst_w;
		u32 src_y = p->src_y + dst_oy * p->src_h / p->dst_h;
		u32 src_w = dst_w * p->src_w / p->dst_w;
		u32 src_h = dst_h * p->src_h / p->dst_h;

		u32 fb_start;
		if (src_w == 0 || src_h == 0 || p->fb_start == 0)
			fb_start = 0;
		else
			fb_start = p->fb_start + 4 * src_x + src_y * p->fb_pitch;

		if (!fb_start) {
			// disable overlay and pipe
			writel_relaxed(0, &de_ui_regs->cfg[0].attr);
#if DE2_RESIZE
			sun8i_ui_scaler_disable(i);
#endif
			continue;
		}

		u32 src_size = SUNXI_DE2_WH(src_w, src_h);
		u32 dst_size = SUNXI_DE2_WH(dst_w, dst_h);
		uint32_t xy = SUNXI_DE2_XY(dst_x, dst_y);

		// overlay
		writel_relaxed(SUNXI_DE2_UI_CFG_ATTR_EN | (2 << 1) | ((255 - (p->alpha % 256)) << 24) | format, &de_ui_regs->cfg[0].attr);
		//writel_relaxed(SUNXI_DE2_UI_CFG_ATTR_EN | format, &de_ui_regs->cfg[0].attr);
		writel_relaxed(fb_start, &de_ui_regs->cfg[0].top_laddr);
		writel_relaxed(p->fb_pitch, &de_ui_regs->cfg[0].pitch);
		writel_relaxed(src_size, &de_ui_regs->cfg[0].size);
		writel_relaxed(src_size, &de_ui_regs->ovl_size);
		writel_relaxed(0, &de_ui_regs->cfg[0].coord);

		// blender output
		if (i == 0) {
			writel_relaxed(dst_size, &de_bld_regs->output_size);
			writel_relaxed(dst_size, &de_glb_regs->size);
		}

		// blender input pipe
		route_reg |= (i + 1) << (pipe * 4);
		fcolor_ctl |= 1 << (8 + pipe);
		if (pipe == 0)
			fcolor_ctl |= 1 << (pipe);
		writel_relaxed(dst_size, &de_bld_regs->attr[pipe].insize);
		writel_relaxed(0xff000000, &de_bld_regs->attr[pipe].fcolor);
		writel_relaxed(xy, &de_bld_regs->attr[pipe].offset);
		writel_relaxed(0x03010301, &de_bld_regs->bld_mode[pipe]);

		pipe++;

#if DE2_RESIZE
		if (src_w != dst_w || src_h != dst_h) {
			// 0x00010000 = 1:1
			u32 hscale = (0x10000 << 4) * src_w / dst_w;
			u32 vscale = (0x10000 << 4) * src_h / dst_h;

			sun8i_ui_scaler_setup(i, src_size, dst_size,
					      hscale, vscale);
		} else {
			sun8i_ui_scaler_disable(i);
		}
#endif
	}

	writel(route_reg, &de_bld_regs->route);
	writel(fcolor_ctl, &de_bld_regs->fcolor_ctl);

	/* apply settings */
	writel(1, &de_glb_regs->dbuff);
}

// }}}
