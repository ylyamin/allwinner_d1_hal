#ifndef TCON_LCD_h_
#define TCON_LCD_h_
#include "platform.h"

typedef struct {
	volatile uint32_t res0[20];		
	volatile uint32_t dphy_ana1;	
	volatile uint32_t dphy_ana2;
	volatile uint32_t dphy_ana3;
	volatile uint32_t dphy_ana4;
	volatile uint32_t res1[4+4+24+4+8]; 
	volatile uint32_t combo_phy_reg0;   
	volatile uint32_t combo_phy_reg1;
	volatile uint32_t combo_phy_reg2;
} DSICOMBO_t;

#define DSI0_PHY    ((DSICOMBO_t *) (DSI_DPHY_BASE))

enum lcd_type
{
	RGB,
	DSI,
};

typedef struct{
	uint32_t lcd_type;
	uint32_t pixclk;
	uint32_t lcd_w;
	uint32_t lcd_h;
	uint32_t hbp;
	uint32_t ht;
	uint32_t hspw;
	uint32_t vbp;
	uint32_t vt;
	uint32_t vspw;
} timing_t;

void tcon_lcd_init(timing_t timing);
void tcon_lcd_enable(void);
void tcon_lcd_disable(void);
#endif
