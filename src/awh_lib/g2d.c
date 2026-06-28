#include "platform.h"
#include "ccu.h"
#include "log.h"
#include "de.h"
#include "irq.h"
#include "g2d.h"

int g2d_rot_finished = 0;

static void g2d_int_handler(void *arg)
{
	(void)arg;

    if (G2D_ROT->ROT_INT & BV(0)) 
    {
        g2d_rot_finished = 1;
        G2D_ROT->ROT_INT |= (0x1 << 0);    
    }
}

int g2d_rot_finish(void)
{  
    return g2d_rot_finished;
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

 	irq_assign(G2D_IRQn, (void *) g2d_int_handler);
	irq_enable(G2D_IRQn);

/*     
    G2D_MIXER->G2D_MIXER_INTERRUPT = 0x10;
    G2D_MIXER->G2D_MIXER_CTRL |= (0x1 << 31);
 */
}



void g2d_rot(struct g2d_rot_t g2d_rot_config)
{
    g2d_rot_finished = 0;    

//Input
    G2D_ROT->ROT_IFMT = g2d_rot_config.fmt;      
    G2D_ROT->ROT_ISIZE = ((g2d_rot_config.src_h-1 ) << 16) | (g2d_rot_config.src_w-1); 
    G2D_ROT->ROT_IPITCH0 = (fmt_to_pitch(g2d_rot_config.fmt) * g2d_rot_config.src_w);                    
    G2D_ROT->ROT_ILADD0 =  (uint32_t) g2d_rot_config.src_fb;                  
//Output
    G2D_ROT->ROT_OSIZE = ((g2d_rot_config.dst_h-1) << 16) | (g2d_rot_config.dst_w-1);                     
    G2D_ROT->ROT_OPITCH0 = (fmt_to_pitch(g2d_rot_config.fmt) * g2d_rot_config.dst_w);                     
    G2D_ROT->ROT_OLADD0 =  (uint32_t) g2d_rot_config.dst_fb; 
//Control
    G2D_ROT->ROT_INT |= (0x1 << 16);
    G2D_ROT->ROT_CTL |= (0x1 << 31) | (g2d_rot_config.hflip << 7) | (g2d_rot_config.vflip << 6) | (g2d_rot_config.rot_angle << 4) | (0x1 << 0);
}

