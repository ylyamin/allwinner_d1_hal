#include "platform.h"
#include <stdio.h>
#include "ccu.h"
#include "gpio.h"
#include "uart.h"
#include "log.h"
#include "irq.h"
#include "de.h"
#include "tcon_lcd.h"

void tcon_find_clock(uint32_t tgt_freq)
{

	uint32_t osc = ccu_clk_hosc_get();
	uint32_t best_n = 12;
	uint32_t best_m = 1;
//	uint32_t best_d = 6;
	uint32_t best_err = 0xffffffff;

	LOG_D("tcon: looking up pll parameters for %dHz", tgt_freq);
	// TODO: why 2x ?
	tgt_freq *= 2;

	for (uint32_t n = 12; n < 100; n ++) {
		for (uint32_t m = 1; m < 3; m++) {
			/*for (uint32_t d = 6; d < 128; d ++) */{
				uint32_t freq = osc * n / m;
				//uint32_t freq = osc * n / m / d;
				
				uint32_t err = ABS(freq - tgt_freq);
				if (err < best_err) {
					best_n = n;
					best_m = m;
					//best_d = d;
					best_err = err;

					if (err == 0) {
						goto end;			
					}
				}
			}
		}
	}
end:
	
	LOG_D("tcon: best: n=%d m=%d err=%d", best_n, best_m, best_err);
	ccu_video0_pll_set(best_n, best_m);
	ccu_tcon_set_video0x4_div(1);
	ccu_tcon_lcd_enable();
}

void tcon_dither(void)
{
	TCON_LCD0->LCD_FRM_SEED_REG[0] = 0x11111111;
	TCON_LCD0->LCD_FRM_SEED_REG[1] = 0x11111111;
	TCON_LCD0->LCD_FRM_SEED_REG[2] = 0x11111111;
	TCON_LCD0->LCD_FRM_SEED_REG[3] = 0x11111111;
	TCON_LCD0->LCD_FRM_SEED_REG[4] = 0x11111111;
	TCON_LCD0->LCD_FRM_SEED_REG[5] = 0x11111111;
	TCON_LCD0->LCD_FRM_TAB_REG[0] = 0x01010000;
	TCON_LCD0->LCD_FRM_TAB_REG[1] = 0x15151111;
	TCON_LCD0->LCD_FRM_TAB_REG[2] = 0x57575555;
	TCON_LCD0->LCD_FRM_TAB_REG[3] = 0x7f7f7777;
	TCON_LCD0->LCD_FRM_CTL_REG = BV(31);
}

static void tcon_int_handler(void *arg)
{
	(void)arg;
	LOG_D("tcon int handler");

	uint32_t gint0 = TCON_LCD0->LCD_GINT0_REG;

	if (gint0 & BV(15)) {
		TCON_LCD0->LCD_GINT0_REG = BV(15);

		de_int_vblank();
	}

	if (gint0 & BV(13)) {
		TCON_LCD0->LCD_GINT0_REG = BV(13);

		//de_int_vblank();

	}
}

void tcon_lcd_init(timing_t timing)
{
	LOG_D("tcon: init");
	tcon_lcd_disable();

// Step 1 Select HV interface type 

if (timing.lcd_type == RGB) 
{
	TCON_LCD0->LCD_CTL_REG &= ~(0 << 24);			// HV(Sync+DE); 
}

if (timing.lcd_type == DSI)
{
	//TCON_LCD0->LCD_CTL_REG |= (1 << 24);			// 8080 I/F - not appled here by some reason
}

	TCON_LCD0->LCD_CTL_REG &= ~BV(0);			// src = DE/color/grayscale/
	TCON_LCD0->LCD_HV_IF_REG &= ~(0x0 << 28); 	// 24bit/1cycle parallel mode

// Step 2 Clock configuration 
	uint32_t tcon_div = 6; /// 6 or 7 ?
	tcon_find_clock(timing.pixclk * tcon_div);

/* 	ccu_video0_pll_set(27, 2);
	ccu_tcon_set_video0x4_div(1);
	ccu_tcon_lcd_enable(); */

	//TCON_LCD0->LCD_CTL_REG |= (1 << 24);			// 8080 I/F - apply here


	TCON_LCD0->LCD_DCLK_REG = tcon_div * 2;// !!!!!!
	//TCON_LCD0->LCD_DCLK_REG = 4;
	TCON_LCD0->LCD_DCLK_REG |= (0x0f << 28);
	delay_us(20);

	// TODO: where does this 2 come from ?
	//LOG_D("tcon_lcd: tcon clk = %dHz pixclk = %dHz", ccu_tcon_get_clk() / tcon_div / 2, timing.pixclk);
	LOG_D("tcon_lcd: tcon clk = %dHz pixclk = %dHz", ccu_tcon_get_clk(), timing.pixclk);

if (timing.lcd_type == DSI) 
{	
	ccu_dsi_enable(); //600Mhz
/* 
	CCU->DSI_CLK_REG = (1 << 24) | (4 << 0);
	CCU->DSI_CLK_REG |= BV(31);
	CCU->DSI_BGR_REG |= BV(16);
	CCU->DSI_BGR_REG |= BV(0); */
}
// ?? init iface
	uint32_t val = timing.vt - timing.lcd_h - 8;
	if (val > 31) val = 31;
	if (val < 10) val = 10;
	TCON_LCD0->LCD_CTL_REG |= ((val & 0x1f) << 4);

//Step 3 Set sequence parameters 

	// init timing
	TCON_LCD0->LCD_BASIC0_REG = ((timing.lcd_w  - 1) << 16) | (timing.lcd_h - 1);
 	TCON_LCD0->LCD_BASIC1_REG = ((timing.ht - 1) << 16) | (timing.hbp - 1);
	TCON_LCD0->LCD_BASIC2_REG = ((timing.vt * 2) << 16) | (timing.vbp - 1);
	TCON_LCD0->LCD_BASIC3_REG = ((timing.hspw)   << 16) | (timing.vspw); 

//Step 4 Open IO output
	// io polarity for h,v,de,clk
	TCON_LCD0->LCD_IO_TRI_REG = 0; // default is 0xffffff (very bad :-)
	TCON_LCD0->LCD_IO_POL_REG = 0; // 2/3phase offset ?! why ?

//Step 5 LVDS digital logic configuration 
//setup_lvds()

if (timing.lcd_type == DSI) 
{	
/* 	TCON_LCD0->LCD_LVDS_IF_REG &= ~BV(27); 
	TCON_LCD0->LCD_LVDS_IF_REG |= BV(20); //! LVDS_MODE_JEIDA  LVDS_18BIT, BV(30) dual
	TCON_LCD0->LCD_LVDS_IF_REG |= BV(31);
	TCON_LCD0->LVDS1_IF_REG = TCON_LCD0->LCD_LVDS_IF_REG; */

//Step 6 LVDS controller configuration
// TCON  LCD0  PHY0 is controlled by COMBO_PHY_REG  (reg0x1110,  reg0x1114)
//enable_lvds();

 	DSI0_PHY->combo_phy_reg1 = 0x43;
	DSI0_PHY->combo_phy_reg0 = 0x1;
	delay_ms(1);
	DSI0_PHY->combo_phy_reg0 = 0x5;
	delay_ms(1);
	DSI0_PHY->combo_phy_reg0 = 0x7;
	delay_ms(1);
	DSI0_PHY->combo_phy_reg0 = 0xf;
	DSI0_PHY->dphy_ana4 = 0x84000000;
	DSI0_PHY->dphy_ana3 = 0x01040000;
	DSI0_PHY->dphy_ana2 &= (0 << 1);
	DSI0_PHY->dphy_ana1 = 0;


/* 	DSI0_PHY->combo_phy_reg1 = 0x00000000;
	DSI0_PHY->combo_phy_reg0 = 0x0000000b;
	DSI0_PHY->dphy_ana4 = 0x844635ee;
	DSI0_PHY->dphy_ana3 = 0xff040000;
	DSI0_PHY->dphy_ana2 = 0x0f000012;
	DSI0_PHY->dphy_ana1 = 0x80000000;
 */

// TCON  LCD0  PHY1 is controlled by LCD_LVDS0_ANA_REG (reg0x220)

/* 	TCON_LCD0->LCD_LVDS_ANA_REG[0] =
		(0x0F << 20) |	// When LVDS signal is 18-bit, LVDS_HPREN_DRV=0x7; when LVDS signal is 24-bit, LVDS_HPREN_DRV=0xF;
		(1 << 24) |	// LVDS_HPREN_DRVC
		(0x04 << 17) |	// Configure LVDS0_REG_C (differential mode voltage) to 4; 100: 336 mV
		(3 << 8) |	// ?LVDS_REG_R Configure LVDS0_REG_V (common mode voltage) to 3;
		0;

	TCON_LCD0->LCD_LVDS_ANA_REG[0] |= (1 << 30);	// en_ldo
	delay_ms(1);
	// 	Lastly, start module voltage, and enable EN_LVDS and EN_24M.
	TCON_LCD0->LCD_LVDS_ANA_REG[0] |= (1 << 31);	// ?LVDS_EN_MB start module voltage
	delay_ms(1);
	TCON_LCD0->LCD_LVDS_ANA_REG[0] |= (1 << 29);	// enable EN_LVDS
	delay_ms(1);
	TCON_LCD0->LCD_LVDS_ANA_REG[0] |= (1 << 28);	// EN_24M
	delay_ms(1); */
}
//Step 5-7 Set and open interrupt function
	//TCON_LCD0->LCD_GINT0_REG = BV(29); //V interrupt
	//TCON_LCD0->LCD_GINT1_REG = line << 16; // Line interrupt 

	irq_assign(LCD_IRQn, (void *) tcon_int_handler);
	irq_enable(LCD_IRQn);

	//tcon_dither();
	LOG_D("tcon: init done");
}

void tcon_lcd_enable(void)
{
//Step 6-8 Open module enable
	TCON_LCD0->LCD_CTL_REG |= BV(31);
	TCON_LCD0->LCD_GCTL_REG |= BV(31);
}

void tcon_lcd_disable(void)
{
	TCON_LCD0->LCD_CTL_REG = 0;
	TCON_LCD0->LCD_GCTL_REG &= ~BV(31);
}


void tcon_dump_regs(void)
{

small_printf("\n\rStep 1 Select HV interface type\n\n\r");

small_printf("	TCON_LCD0->LCD_CTL_REG		0x040 	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x040)	);
small_printf("	TCON_LCD0->LCD_HV_IF_REG	0x058	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x058)	);

small_printf("\n\rStep 2 Clock configuration\n\n\r");

small_printf("	CCU->PLL_PERI_CTRL_REG		0x020	%08x\n\r",  *(uint32_t *)( 0x02001000 + 0x020)	);
small_printf("	CCU->PLL_VIDEO0_CTRL_REG	0x040	%08x\n\r",  *(uint32_t *)( 0x02001000 + 0x040)	);
small_printf("	CCU->TCONLCD_CLK_REG		0xB60	%08x\n\r",  *(uint32_t *)( 0x02001000 + 0xB60)	);
small_printf("	CCU->TCONLCD_BGR_REG		0xB7C	%08x\n\r",  *(uint32_t *)( 0x02001000 + 0xB7C)	);
small_printf("	TCON_LCD0->LCD_DCLK_REG		0x044	%08x\n\r",  *(uint32_t *)( 0x05461000 + 0x044)	);
small_printf("	CCU->DSI_CLK_REG			0xB24	%08x\n\r",  *(uint32_t *)( 0x02001000 + 0xB24)		);
small_printf("	CCU->DSI_BGR_REG			0xB4C	%08x\n\r",  *(uint32_t *)( 0x02001000 + 0xB4C)		);
small_printf("	CCU->LVDS_BGR_REG			0xBAC	%08x\n\r",  *(uint32_t *)( 0x02001000 + 0xBAC)		);

small_printf("\n\rStep 3 Set sequence parameters\n\n\r");

small_printf("	TCON_LCD0->LCD_BASIC0_REG	0x048	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x048)	);	
small_printf("	TCON_LCD0->LCD_BASIC1_REG	0x04C	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x04C)	);		
small_printf("	TCON_LCD0->LCD_BASIC2_REG	0x050	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x050)	);		
small_printf("	TCON_LCD0->LCD_BASIC3_REG	0x054	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x054)	);	

small_printf("\n\rStep 4 Open IO output\n\n\r");		

small_printf("	TCON_LCD0->LCD_IO_TRI_REG	0x08C	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x08C) 	);			
small_printf("	TCON_LCD0->LCD_IO_POL_REG	0x088	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x088) 	);		

small_printf("\n\rStep 5 LVDS digital logic configuration\n\n\r");	

small_printf("	TCON_LCD0->LCD_LVDS_IF_REG	0x084	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x084)	);				
small_printf("	TCON_LCD0->LVDS1_IF_REG		0x244	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x244)	); 		

small_printf("\n\rStep 6 LVDS controller configuration\n\n\r");

small_printf("PHY0 COMBO_PHY_REG \n\n\r");

small_printf("	DSI0_PHY->combo_phy_reg1	0x114	%08x\n\r", *(uint32_t *)( 0x05451000 + 0x114)	); 			
small_printf("	DSI0_PHY->combo_phy_reg0	0x110	%08x\n\r", *(uint32_t *)( 0x05451000 + 0x110)	); 			
small_printf("	DSI0_PHY->dphy_ana4			0x05c	%08x\n\r", *(uint32_t *)( 0x05451000 + 0x05c)		); 			
small_printf("	DSI0_PHY->dphy_ana3			0x058	%08x\n\r", *(uint32_t *)( 0x05451000 + 0x058)		); 			
small_printf("	DSI0_PHY->dphy_ana2			0x054	%08x\n\r", *(uint32_t *)( 0x05451000 + 0x054)		); 		
small_printf("	DSI0_PHY->dphy_ana1			0x050	%08x\n\r", *(uint32_t *)( 0x05451000 + 0x050)		); 		

small_printf("\n\rPHY1 LCD_LVDS0_ANA_REG \n\n\r");					

small_printf("	TCON_LCD0->LCD_LVDS_ANA_REG[0]	0x220	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x220) );		

small_printf("\n\rStep 5-7 Set and open interrupt function\n\n\r");

small_printf("	TCON_LCD0->LCD_GINT0_REG	0x004	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x004) 	);		
small_printf("	TCON_LCD0->LCD_GINT1_REG	0x008	%08x\n\r", *(uint32_t *)( 0x05461000 + 0x008) 	);	

}