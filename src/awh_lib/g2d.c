#include "platform.h"
#include "ccu.h"
#include "log.h"
#include "de.h"
#include "irq.h"

extern struct layer_t layers[1];

static void g2d_int_handler(void *arg)
{
	(void)arg;
    G2D_ROT->ROT_INT |= (0x1 << 0);    
    G2D_MIXER->G2D_MIXER_INTERRUPT |= (0x1 << 0);     
}

void g2d_init(void)
{
	ccu_g2d_set_peripx2_div(4); // (1.2Gz / 4)
	ccu_g2d_enable();

	G2D_TOP->G2D_SCLK_GATE = 0x3;
	G2D_TOP->G2D_HCLK_GATE = 0x3;        
    G2D_TOP->G2D_SCLK_DIV = (0x0 <<4) | (0x0 << 0);
	G2D_TOP->G2D_AHB_RST   = 0x0;
	G2D_TOP->G2D_AHB_RST   = 0x3;

/*  	irq_assign(G2D_IRQn, (void *) g2d_int_handler);
	irq_enable(G2D_IRQn);  */

/*     G2D_MIXER->G2D_MIXER_INTERRUPT = 0x10;
    G2D_MIXER->G2D_MIXER_CTRL |= (0x1 << 31);
 */
}

void g2d_rot(void *src_fb, void *dst_fb)
{
                 
//Input
    G2D_ROT->ROT_IFMT = layers[0].fmt;      
    G2D_ROT->ROT_ISIZE = ((200-1) << 16) | (320-1); 
    G2D_ROT->ROT_IPITCH0 = fmt_to_pitch(layers[0].fmt) * 320;                    
    G2D_ROT->ROT_ILADD0 = (uint32_t)src_fb;                  
//Output
    G2D_ROT->ROT_OSIZE = ((320-1) << 16) | (200-1);                     
    G2D_ROT->ROT_OPITCH0 = fmt_to_pitch(layers[0].fmt) * 200;                   
    G2D_ROT->ROT_OLADD0 = (uint32_t)dst_fb;     
//Control
    G2D_ROT->ROT_CTL |= (0x1 << 31) | (0x3 << 4) | (0x1 << 0); //270 copy   
    G2D_ROT->ROT_INT |= (0x1 << 0) | (0x1 << 16);   
}

