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
#include <config.h>
#include <platform.h>
#include <log.h>
#include <ccu.h>
#include <stdbool.h>

///////////////

extern void * memset(void *m, int c, size_t n);
extern void * memcpy( void *dst0, const void* src0, size_t len0);

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define BITS_PER_LONG 64

#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

/////

#define RISCV_FENCE(p, s) \
        __asm__ __volatile__ ("fence " #p "," #s : : : "memory")

/* These barriers need to enforce ordering on both devices or memory. */
#define mb()            RISCV_FENCE(iorw,iorw)
#define rmb()           RISCV_FENCE(ir,ir)
#define wmb()           RISCV_FENCE(ow,ow)

#define __arch_getl(a)                  (*(unsigned int *)(a))
#define __arch_putl(v, a)               (*(unsigned int *)(a) = (v))

#define dmb() mb()
#define __iormb() rmb()
#define __iowmb() wmb()

static inline void writel(uint32_t val, volatile void *addr)
{
    __iowmb();
    __arch_putl(val, addr);
}

static inline uint32_t readl(const volatile void *addr)
{
    uint32_t val;

    val = __arch_getl(addr);
    __iormb();
    return val;
}
//////////////

#ifndef DUMP_DSI_INIT
#define DUMP_DSI_INIT 0
#endif

#ifndef DSI_FULL_INIT
#define DSI_FULL_INIT 0
#endif

#ifndef DE2_RESIZE
#define DE2_RESIZE 0
#endif

#define DPHY_BASE (0x05450000 + 0x1000)
#define DSI_BASE  (0x05450000)


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

/* 	
	.pixclk = 55000000,
	.w = 480,
	.h = 1280,
	.hbp = 150, //hsync back porch(pixel) + hsync plus width(pixel)
	.ht = 694,  //hsync total cycle(pixel)
	.hspw = 40, //hsync plus width(pixel)

	.vbp = 12,  //vsync back porch(line) + vysnc plus width(line)
	.vt = 1308, //vysnc total cycle(line)
	.vspw = 10, //vysnc plus width(pixel) 

*/

#define PANEL_HDISPLAY		(480)
#define PANEL_HSYNC_START	(480 + 64)
#define PANEL_HSYNC_END		(480 + 40)
#define PANEL_HTOTAL		(480 + 64 + 40 + 110)
#define PANEL_VDISPLAY		(1280)
#define PANEL_VSYNC_START	(1280 + 16)
#define PANEL_VSYNC_END		(1280 + 16 + 10)
#define PANEL_VTOTAL		(1280 + 16 + 10 + 2)
//#define PANEL_CLOCK		(PANEL_HTOTAL * PANEL_VTOTAL * 60 / 1000)
#define PANEL_CLOCK			(55000)
#define PANEL_LANES			4
#define PANEL_DSI_BPP		24
#define PANEL_BURST			0

/*
 * Init sequence was supplied by the panel vendor:
 */

struct dcs_seq {
	uint8_t len;
	const uint8_t *data;
	uint8_t type;
};

static const struct dcs_seq panel_dcs_seq_initlist[] = {
	{2,   {0xF0,0x5A,0x59} },
	{2,   {0xF1,0xA5,0xA6} },
	{14,  {0xB0,0x54,0x32,0x23,0x45,0x44,0x44,0x44,0x44,0x9F,0x00,0x01,0x9F,0x00,0x01} },
	{10,  {0xB1,0x32,0x84,0x02,0x83,0x29,0x06,0x06,0x72,0x06,0x06} },
	{1,   {0xB2,0x73} },
	{20,  {0xB3,0x0B,0x09,0x13,0x11,0x0F,0x0D,0x00,0x00,0x00,0x03,0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x05,0x07} },
	{20,  {0xB4,0x0A,0x08,0x12,0x10,0x0E,0x0C,0x00,0x00,0x00,0x03,0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x04,0x06} },
	{2,   {0xB6,0x13,0x13} },
	{4,   {0xB8,0xB4,0x43,0x02,0xCC} },
	{4,   {0xB9,0xA5,0x20,0xFF,0xC8} },
	{2,   {0xBA,0x88,0x23} },
	{10,  {0xBD,0x43,0x0E,0x0E,0x50,0x50,0x29,0x10,0x03,0x44,0x03} },
	{8,   {0xC1,0x00,0x0C,0x16,0x04,0x00,0x30,0x10,0x04} },
	{2,   {0xC2,0x21,0x81} },
	{2,   {0xC3,0x02,0x30} },
	{2,   {0xC7,0x25,0x6A} },
	{38,  {0xC8,0x7C,0x68,0x59,0x4E,0x4B,0x3C,0x41,0x2B,0x44,0x43,0x43,0x60,0x4E,0x55,0x47,0x44,0x38,0x27,0x06,0x7C,0x68,0x59,0x4E,0x4B,0x3C,0x41,0x2B,0x44,0x43,0x43,0x60,0x4E,0x55,0x47,0x44,0x38,0x27,0x06} },
	{6,   {0xD4,0x00,0x00,0x00,0x32,0x04,0x51} },
	{2,   {0xF1,0x5A,0x59} },
	{2,   {0xF0,0xA5,0xA6} },
	{1,   {0x36,0x14} },
	{1,   {0x35,0x00} },
	{1,   {0x11,0x00} },
	{ .type = 1, .len = 120 },
	{1,   {0x29,0x00} },	
	{ .type = 1, .len = 20 },
};

ssize_t mipi_dsi_dcs_write(const uint8_t *data, size_t len);

static void panel_init(void)
{
	int i, ret;

	// run panel init

	for (i = 0; i < ARRAY_SIZE(panel_dcs_seq_initlist); i++) {
		const struct dcs_seq* s = &panel_dcs_seq_initlist[i];

		if (s->type) {
			delay_us(s->len * 1000);
			continue;
		}

        ret = mipi_dsi_dcs_write(s->data, s->len);
        if (ret < 0) {
			LOG_D("DCS failed\n");
                        return;
		}
	}
}

// }}}
// {{{ DPHY

#define DPHY_GCTL_REG             0x00
#define DPHY_GCTL_LANE_NUM(n)             ((((n) - 1) & 3) << 4)
#define DPHY_GCTL_EN                      BV(0)

#define DPHY_TX_CTL_REG           0x04
#define DPHY_TX_CTL_HS_TX_CLK_CONT        BV(28)

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
#define DPHY_ANA0_REG_PWS                 BV(31)
#define DPHY_ANA0_REG_DMPC                BV(28)
#define DPHY_ANA0_REG_DMPD(n)             (((n) & 0xf) << 24)
#define DPHY_ANA0_REG_SLV(n)              (((n) & 7) << 12)
#define DPHY_ANA0_REG_DEN(n)              (((n) & 0xf) << 8)

#define DPHY_ANA1_REG             0x50
#define DPHY_ANA1_REG_VTTMODE             BV(31)
#define DPHY_ANA1_REG_CSMPS(n)            (((n) & 3) << 28)
#define DPHY_ANA1_REG_SVTT(n)             (((n) & 0xf) << 24)

#define DPHY_ANA2_REG             0x54
#define DPHY_ANA2_EN_P2S_CPU(n)           (((n) & 0xf) << 24)
#define DPHY_ANA2_EN_P2S_CPU_MASK         GENMASK(27, 24)
#define DPHY_ANA2_EN_CK_CPU               BV(4)
#define DPHY_ANA2_REG_ENIB                BV(1)

#define DPHY_ANA3_REG             0x58
#define DPHY_ANA3_EN_VTTD(n)              (((n) & 0xf) << 28)
#define DPHY_ANA3_EN_VTTD_MASK            GENMASK(31, 28)
#define DPHY_ANA3_EN_VTTC                 BV(27)
#define DPHY_ANA3_EN_DIV                  BV(26)
#define DPHY_ANA3_EN_LDOC                 BV(25)
#define DPHY_ANA3_EN_LDOD                 BV(24)
#define DPHY_ANA3_EN_LDOR                 BV(18)

#define DPHY_ANA4_REG             0x5c
#define DPHY_ANA4_REG_DMPLVC              BV(24)
#define DPHY_ANA4_REG_DMPLVD(n)           (((n) & 0xf) << 20)
#define DPHY_ANA4_REG_CKDV(n)             (((n) & 0x1f) << 12)
#define DPHY_ANA4_REG_TMSC(n)             (((n) & 3) << 10)
#define DPHY_ANA4_REG_TMSD(n)             (((n) & 3) << 8)
#define DPHY_ANA4_REG_TXDNSC(n)           (((n) & 3) << 6)
#define DPHY_ANA4_REG_TXDNSD(n)           (((n) & 3) << 4)
#define DPHY_ANA4_REG_TXPUSC(n)           (((n) & 3) << 2)
#define DPHY_ANA4_REG_TXPUSD(n)           ((n) & 3)

#define DPHY_DBG5_REG             0xf4


static void dphy_write(unsigned long reg, uint32_t val)
{
	writel(val, DPHY_BASE + reg);
}


static void dphy_update_BVs(unsigned long reg, uint32_t mask, uint32_t val)
{
	uint32_t tmp = readl(DPHY_BASE + reg);
	tmp &= ~mask;
	writel(tmp | val, DPHY_BASE + reg);
}

static void dphy_enable(void)
{
        uint8_t lanes_mask = GENMASK(PANEL_LANES - 1, 0);
/* 
	// 150MHz (600 / 4)
	writel((1 << 15 // enable ) |
	       (2 << 8 // src: 0=video0(1x) 2=periph0(1x) ) |
	       ((4 - 1) << 0) // M-1 ,
	       CCU_MIPI_DSI_CLK);
 */
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
	delay_us(5);

	dphy_write(DPHY_ANA3_REG,
		   DPHY_ANA3_EN_LDOR |
		   DPHY_ANA3_EN_LDOC |
		   DPHY_ANA3_EN_LDOD);
	delay_us(1);

	dphy_update_BVs(DPHY_ANA3_REG,
			 DPHY_ANA3_EN_VTTC |
			 DPHY_ANA3_EN_VTTD_MASK,
			 DPHY_ANA3_EN_VTTC |
			 DPHY_ANA3_EN_VTTD(lanes_mask));
	delay_us(1);

	dphy_update_BVs(DPHY_ANA3_REG,
			 DPHY_ANA3_EN_DIV,
			 DPHY_ANA3_EN_DIV);
	delay_us(1);

	dphy_update_BVs(DPHY_ANA2_REG,
			 DPHY_ANA2_EN_CK_CPU,
			 DPHY_ANA2_EN_CK_CPU);
	delay_us(1);

	dphy_update_BVs(DPHY_ANA1_REG,
			 DPHY_ANA1_REG_VTTMODE,
			 DPHY_ANA1_REG_VTTMODE);

	dphy_update_BVs(DPHY_ANA2_REG,
			 DPHY_ANA2_EN_P2S_CPU_MASK,
			 DPHY_ANA2_EN_P2S_CPU(lanes_mask));
}

// }}}
// {{{ DSI

#define SUN6I_DSI_CTL_REG               0x000
#define SUN6I_DSI_CTL_EN                        BV(0)

#define SUN6I_DSI_BASIC_CTL_REG         0x00c
#define SUN6I_DSI_BASIC_CTL_TRAIL_INV(n)                (((n) & 0xf) << 4)
#define SUN6I_DSI_BASIC_CTL_TRAIL_FILL          BV(3)
#define SUN6I_DSI_BASIC_CTL_HBP_DIS             BV(2)
#define SUN6I_DSI_BASIC_CTL_HSA_HSE_DIS         BV(1)
#define SUN6I_DSI_BASIC_CTL_VIDEO_BURST         BV(0)

#define SUN6I_DSI_BASIC_CTL0_REG        0x010
#define SUN6I_DSI_BASIC_CTL0_HS_EOTP_EN         BV(18)
#define SUN6I_DSI_BASIC_CTL0_CRC_EN             BV(17)
#define SUN6I_DSI_BASIC_CTL0_ECC_EN             BV(16)
#define SUN6I_DSI_BASIC_CTL0_INST_ST            BV(0)

#define SUN6I_DSI_BASIC_CTL1_REG        0x014
#define SUN6I_DSI_BASIC_CTL1_VIDEO_ST_DELAY(n)  (((n) & 0x1fff) << 4)
#define SUN6I_DSI_BASIC_CTL1_VIDEO_FILL         BV(2)
#define SUN6I_DSI_BASIC_CTL1_VIDEO_PRECISION    BV(1)
#define SUN6I_DSI_BASIC_CTL1_VIDEO_MODE         BV(0)

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
#define SUN6I_DSI_INST_FUNC_LANE_CEN            BV(4)
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
#define SUN6I_DSI_TCON_DRQ_ENABLE_MODE          BV(28)
#define SUN6I_DSI_TCON_DRQ_SET(n)               ((n) & 0x3ff)

#define SUN6I_DSI_PIXEL_CTL0_REG        0x080
#define SUN6I_DSI_PIXEL_CTL0_PD_PLUG_DISABLE    BV(16)
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
#define SUN6I_DSI_CMD_CTL_RX_OVERFLOW           BV(26)
#define SUN6I_DSI_CMD_CTL_RX_FLAG               BV(25)
#define SUN6I_DSI_CMD_CTL_TX_FLAG               BV(9)

#define SUN6I_DSI_CMD_RX_REG(n)         (0x240 + (n) * 0x04)

#define SUN6I_DSI_DEBUG_DATA_REG        0x2f8

#define SUN6I_DSI_CMD_TX_REG(n)         (0x300 + (n) * 0x04)

#define SUN6I_DSI_SYNC_POINT            40

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

static const uint32_t sun6i_dsi_ecc_array[] = {
        [0] = (BV(0) | BV(1) | BV(2) | BV(4) | BV(5) | BV(7) | BV(10) |
               BV(11) | BV(13) | BV(16) | BV(20) | BV(21) | BV(22) |
               BV(23)),
        [1] = (BV(0) | BV(1) | BV(3) | BV(4) | BV(6) | BV(8) | BV(10) |
               BV(12) | BV(14) | BV(17) | BV(20) | BV(21) | BV(22) |
               BV(23)),
        [2] = (BV(0) | BV(2) | BV(3) | BV(5) | BV(6) | BV(9) | BV(11) |
               BV(12) | BV(15) | BV(18) | BV(20) | BV(21) | BV(22)),
        [3] = (BV(1) | BV(2) | BV(3) | BV(7) | BV(8) | BV(9) | BV(13) |
               BV(14) | BV(15) | BV(19) | BV(20) | BV(21) | BV(23)),
        [4] = (BV(4) | BV(5) | BV(6) | BV(7) | BV(8) | BV(9) | BV(16) |
               BV(17) | BV(18) | BV(19) | BV(20) | BV(22) | BV(23)),
        [5] = (BV(10) | BV(11) | BV(12) | BV(13) | BV(14) | BV(15) |
               BV(16) | BV(17) | BV(18) | BV(19) | BV(21) | BV(22) |
               BV(23)),
};

static uint32_t sun6i_dsi_ecc_compute(unsigned int data)
{
        int i;
        uint8_t ecc = 0;

        for (i = 0; i < ARRAY_SIZE(sun6i_dsi_ecc_array); i++) {
                uint32_t field = sun6i_dsi_ecc_array[i];
                bool init = false;
                uint8_t val = 0;
                int j;

                for (j = 0; j < 24; j++) {
                        if (!(BV(j) & field))
                                continue;

                        if (!init) {
                                val = (BV(j) & data) ? 1 : 0;
                                init = true;
                        } else {
                                val ^= (BV(j) & data) ? 1 : 0;
                        }
                }

                ecc |= val << i;
        }

        return ecc;
}

static uint16_t const crc_ccitt_table[256] = {
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

static inline uint16_t crc_ccitt_byte(uint16_t crc, const uint8_t c)
{
	return (crc >> 8) ^ crc_ccitt_table[(crc ^ c) & 0xff];
}

static uint16_t crc_ccitt(uint16_t crc, uint8_t const *buffer, size_t len)
{
	while (len--)
		crc = crc_ccitt_byte(crc, *buffer++);
	return crc;
}

static uint16_t sun6i_dsi_crc_compute(uint8_t const *buffer, size_t len)
{
        return crc_ccitt(0xffff, buffer, len);
}

static uint16_t sun6i_dsi_crc_repeat(uint8_t pd, uint8_t *buffer, size_t len)
{
        memset(buffer, pd, len);

        return sun6i_dsi_crc_compute(buffer, len);
}

static uint32_t sun6i_dsi_build_sync_pkt(uint8_t dt, uint8_t vc, uint8_t d0, uint8_t d1)
{
        uint32_t val = dt & 0x3f;

        val |= (vc & 3) << 6;
        val |= (d0 & 0xff) << 8;
        val |= (d1 & 0xff) << 16;
        val |= sun6i_dsi_ecc_compute(val) << 24;

        return val;
}

static uint32_t sun6i_dsi_build_blk0_pkt(uint8_t vc, uint16_t wc)
{
        return sun6i_dsi_build_sync_pkt(MIPI_DSI_BLANKING_PACKET, vc,
                                        wc & 0xff, wc >> 8);
}

static uint32_t sun6i_dsi_build_blk1_pkt(uint16_t pd, uint8_t *buffer, size_t len)
{
        uint32_t val = SUN6I_DSI_BLK_PD(pd);

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
		LOG_D("\t{ 0x%04lx, 0x%08x },\n", reg, val);
#endif
	writel(val, DSI_BASE + reg);
}

static void dsi_update_BVs(unsigned long reg, uint32_t mask, uint32_t val)
{
	uint32_t tmp = readl(DSI_BASE + reg);
#if DUMP_DSI_INIT
	if (reg == 0x10 && mask == 1 && val == 1) {
		LOG_D("\t{ MAGIC_COMMIT, 0 },\n");
	} else {
		LOG_D("dsi_update_BVs: 0x%08lx : %08x -> (%08x) %08x\n", DSI_BASE + reg, tmp, mask, val);
	}
#endif
	tmp &= ~mask;
	writel(tmp | val, DSI_BASE + reg);
}

static void sun6i_dsi_inst_setup(enum sun6i_dsi_inst_id id,
				 enum sun6i_dsi_inst_mode mode,
				 bool clock, uint8_t data,
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
        uint8_t lanes_mask = GENMASK(PANEL_LANES - 1, 0);

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

static uint16_t sun6i_dsi_get_line_num(void)
{
	unsigned int Bpp = PANEL_DSI_BPP / 8;

	return PANEL_HTOTAL * Bpp / PANEL_LANES;
}

#define SUN6I_DSI_TCON_DIV	6

static uint16_t sun6i_dsi_get_drq_edge0(uint16_t line_num, uint16_t edge1)
{
	uint16_t edge0 = edge1;

	edge0 += (PANEL_HDISPLAY + 40) * SUN6I_DSI_TCON_DIV / 8;

	if (edge0 > line_num)
		return edge0 - line_num;

	return 1;
}

static uint16_t sun6i_dsi_get_drq_edge1(uint16_t line_num)
{
	unsigned int Bpp = PANEL_DSI_BPP / 8;
	unsigned int hbp = PANEL_HTOTAL - PANEL_HSYNC_END;
	uint16_t edge1;

	edge1 = SUN6I_DSI_SYNC_POINT;
	edge1 += (PANEL_HDISPLAY + hbp + 20) * Bpp / PANEL_LANES;

	if (edge1 > line_num)
		return line_num;

	return edge1;
}

static void sun6i_dsi_setup_burst(void)
{
        uint32_t val = 0;

        if (PANEL_BURST) {
                uint16_t line_num = sun6i_dsi_get_line_num();
                uint16_t edge0, edge1;

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
                uint16_t drq = (PANEL_HSYNC_START - PANEL_HDISPLAY) - 20;

                drq *= PANEL_DSI_BPP;
                drq /= 32;

                val = (SUN6I_DSI_TCON_DRQ_ENABLE_MODE |
                       SUN6I_DSI_TCON_DRQ_SET(drq));
        }

        dsi_write(SUN6I_DSI_TCON_DRQ_REG, val);
}

static void sun6i_dsi_setup_inst_loop(void)
{
        uint16_t delay = 50 - 1;

        if (PANEL_BURST) {
                uint32_t hsync_porch = (PANEL_HTOTAL - PANEL_HDISPLAY) * 150;

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
        uint32_t val = SUN6I_DSI_PIXEL_PH_VC(channel);
        uint8_t dt, fmt;
        uint16_t wc;

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
        uint16_t hbp = 0, hfp = 0, hsa = 0, hblk = 0, vblk = 0;
        uint32_t basic_ctl = 0;
        size_t bytes;
        uint8_t *buffer;

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
	
	//buffer = malloc(bytes);
	uint8_t malloc[bytes] = {};
	buffer = &malloc[0];

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

static uint16_t sun6i_dsi_get_video_start_delay(void)
{
        uint16_t delay = PANEL_VTOTAL - (PANEL_VSYNC_START - PANEL_VDISPLAY) + 1;
        if (delay > PANEL_VTOTAL)
                delay = delay % PANEL_VTOTAL;
	if (delay < 1)
		delay = 1;

        return delay;
}

static void sun6i_dsi_inst_abort(void)
{
	dsi_update_BVs(SUN6I_DSI_BASIC_CTL0_REG,
			SUN6I_DSI_BASIC_CTL0_INST_ST, 0);
}

static void sun6i_dsi_inst_commit(void)
{
	dsi_update_BVs(SUN6I_DSI_BASIC_CTL0_REG,
			SUN6I_DSI_BASIC_CTL0_INST_ST,
			SUN6I_DSI_BASIC_CTL0_INST_ST);
}

static int sun6i_dsi_inst_wait_for_completion(void)
{
	ulong end_ts = get_time_us() + 5000;

        while (dsi_read(SUN6I_DSI_BASIC_CTL0_REG) & SUN6I_DSI_BASIC_CTL0_INST_ST) {
		if (end_ts < get_time_us())
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
		dsi_update_BVs(SUN6I_DSI_INST_FUNC_REG(DSI_INST_ID_LP11),
				SUN6I_DSI_INST_FUNC_LANE_CEN, 0);

        return 0;
}

static uint32_t sun6i_dsi_dcs_build_pkt_hdr(uint8_t type, const uint8_t* buf, unsigned len)
{
        uint32_t pkt = type;

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

ssize_t mipi_dsi_dcs_write(const uint8_t *data, size_t len)
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
		uint8_t *bounce;
		uint16_t crc;

		dsi_write(SUN6I_DSI_CMD_TX_REG(0),
			  sun6i_dsi_dcs_build_pkt_hdr(MIPI_DSI_DCS_LONG_WRITE, data, len));
		
		//bounce = zalloc(len + sizeof(crc) + 4);
		uint8_t zalloc[len + sizeof(crc) + 4] = {};
		bounce = &zalloc[0];

		memcpy(bounce, data, len);
		bounce_len += len;

		crc = sun6i_dsi_crc_compute(bounce, len);
		memcpy(bounce + bounce_len, &crc, sizeof(crc));
		bounce_len += sizeof(crc);

		for (unsigned i = 0; i < DIV_ROUND_UP(bounce_len, 4); i++)
			dsi_write(SUN6I_DSI_CMD_TX_REG(1 + i), ((uint32_t*)bounce)[i]);

		dsi_write(SUN6I_DSI_CMD_CTL_REG, bounce_len + 4 - 1);

		sun6i_dsi_start(DSI_START_LPTX);
		/*
		 * TODO: There's some BVs (reg 0x200, BVs 8/9) that
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

void dsi_init(void)
{
        /*
         * Enable the DSI block.
         */
#if DUMP_DSI_INIT
	LOG_D("struct reg_inst dsi_init_seq[] = {\n");
#endif
        dsi_write(SUN6I_DSI_CTL_REG, SUN6I_DSI_CTL_EN);

        dsi_write(SUN6I_DSI_BASIC_CTL0_REG,
		  SUN6I_DSI_BASIC_CTL0_ECC_EN | SUN6I_DSI_BASIC_CTL0_CRC_EN);

        dsi_write(SUN6I_DSI_TRANS_START_REG, 10);
        dsi_write(SUN6I_DSI_TRANS_ZERO_REG, 0);

        sun6i_dsi_inst_init();

        dsi_write(SUN6I_DSI_DEBUG_DATA_REG, 0xff);

	uint16_t delay = sun6i_dsi_get_video_start_delay();
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
	LOG_D("};\n");
#endif

	dphy_enable();


#if DUMP_DSI_INIT
	LOG_D("\nstruct reg_inst dsi_panel_init_seq[] = {\n");
#endif
	panel_init();

#if DUMP_DSI_INIT
	LOG_D("};\n");
#endif

        sun6i_dsi_start(DSI_START_HSC);

        delay_us(1000);

        sun6i_dsi_start(DSI_START_HSD);
}
