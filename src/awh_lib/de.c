#include "platform.h"
#include "ccu.h"
#include "uart.h"
#include "log.h"
#include "de.h"
#include "de_priv.h"
#include "de_scaler_table.h"

uint8_t fmtpitch[] = {
	[LAY_FBFMT_ARGB_8888] = 4,
	[LAY_FBFMT_ABGR_8888] = 4,
	[LAY_FBFMT_RGBA_8888] = 4,
	[LAY_FBFMT_BGRA_8888] = 4,
	[LAY_FBFMT_XRGB_8888] = 4,
	[LAY_FBFMT_XBRG_8888] = 4,
	[LAY_FBFMT_RGBX_8888] = 4,
	[LAY_FBFMT_BGRX_8888] = 4,
	[LAY_FBFMT_RGB_888] = 3,
	[LAY_FBFMT_BGR_888] = 3,
	[LAY_FBFMT_RGB_565] = 2,
	[LAY_FBFMT_BGR_565] = 2,
	[LAY_FBFMT_ARGB_4444] = 2,
	[LAY_FBFMT_ABGR_4444] = 2,
	[LAY_FBFMT_RGBA_4444] = 2,
	[LAY_FBFMT_BGRA_4444] = 2,
	[LAY_FBFMT_ARGB_1555] = 2,
	[LAY_FBFMT_ABGR_1555] = 2,
	[LAY_FBFMT_RGBA_5551] = 2,
	[LAY_FBFMT_BGRA_5551] = 2,
};

struct blender_t blender = {
	.lcd_w = 600,
	.lcd_h = 1280,
	.lcd_offset_w = 60,
	.lcd_offset_h = 0,
	.pipe[0] = {
		.pipe_w = 480,
		.pipe_h = 768,
		.pipe_offset_w = 0,
		.pipe_offset_h = 256,
	},
};

struct layer_t layers = {
	.layer[0] = {},
};

uint32_t fmt_to_pitch(uint8_t fmt)
{
	return fmtpitch[fmt];
}

void de_config(struct layer_t new_layers, struct blender_t new_blender)
{
	blender = new_blender;
	layers = new_layers;
}

void de_init(void)
{
	ccu_de_set_peripx2_div(4); // (1.2Gz / 4)
	ccu_de_enable();

	// enable core0
	DE_TOP->SCLK_GATE |= BV(0);
	DE_TOP->HCLK_GATE |= BV(0);
	DE_TOP->AHB_RESET |= BV(0);
	// route core0 to tcon0
	DE_TOP->DE2TCON_MUX = 0;

	// enable mixer
	DE_MUX_GLB->CTL = BV(0); //enable
	DE_MUX_GLB->STS = 0;
	DE_MUX_GLB->DBUFFER = 1; //register value be ready for update  
	DE_MUX_GLB->SIZE = ((blender.lcd_h-1) << 16) | (blender.lcd_w-1);

	// disable all overlay units (and all layers)
	for (uint32_t i = 0; i < 4; i ++) {
		DE_MUX_OVL_V->LAYER[i].ATTCTL = 0;
		DE_MUX_OVL_UI1->LAYER[i].ATTCTL = 0;
		DE_MUX_OVL_UI2->LAYER[i].ATTCTL = 0;
		DE_MUX_OVL_UI3->LAYER[i].ATTCTL = 0;
	}

	// put all processing on bypass
	DE_MUX_VSU->CTRL_REG = 0;
	DE_MUX_GSU1->CTRL_REG = 0;
	DE_MUX_GSU2->CTRL_REG = 0;
	DE_MUX_GSU3->CTRL_REG = 0;
	DE_MUX_FCE->GCTRL_REG = 0;
	DE_MUX_BWS->GCTRL_REG = 0;
	DE_MUX_LTI->EN = 0;
	DE_MUX_PEAK->CTRL_REG = 0;
	DE_MUX_ASE->CTRL_REG = 0;
	DE_MUX_FCC->FCC_CTL_REG = 0;
	DE_MUX_DCSC->BYPASS_REG = 0;
	
	// setup blender
	DE_MUX_BLD->FILLCOLOR_CTL = (blender.pipe[1].pipe_enable << 9) | // 0x0303 enable pipe 0,1 and pipe 0,1 fill color
								(blender.pipe[0].pipe_enable << 8) | 
								(blender.pipe[1].pipe_enable << 1) | 
								(blender.pipe[0].pipe_enable << 0); 

	DE_MUX_BLD->CH_RTCTL = 0x0010; // route channel 0(V) to pipe 0 
								   // route channel 1(UI1) to pipe 1 
								   // seems D1 have only one UI channel and one V channel 'de_feat.h')

	DE_MUX_BLD->PREMUL_CTL = 0; //all alpha data is no-pre-multiply alpha
	DE_MUX_BLD->BK_COLOR = 0xFF00FF; // RGB no alpha
	DE_MUX_BLD->SIZE = ((blender.lcd_h-1) << 16) | (blender.lcd_w-1); // lcd size

	// no color keying 
	DE_MUX_BLD->KEY_CTL = 0;
	DE_MUX_BLD->OUT_COLOR = 0;
	DE_MUX_BLD->CTL[0] = 0x03010301;

	//DE_MUX_BLD->PIPE[0].FILL_COLOR = 0xFF00FF00;
	DE_MUX_BLD->PIPE[0].CH_ISIZE = ((blender.pipe[0].pipe_h-1) << 16) | (blender.pipe[0].pipe_w-1);
	DE_MUX_BLD->PIPE[0].CH_OFFSET = ((blender.lcd_offset_h + blender.pipe[0].pipe_offset_h) << 16) | (blender.lcd_offset_w + blender.pipe[0].pipe_offset_w);
	
 	//DE_MUX_BLD->PIPE[1].FILL_COLOR = 0xFFFF0000;
	DE_MUX_BLD->PIPE[1].CH_ISIZE = ((blender.pipe[1].pipe_h-1) << 16) | (blender.pipe[1].pipe_w-1);
	DE_MUX_BLD->PIPE[1].CH_OFFSET = ((blender.lcd_offset_h + blender.pipe[1].pipe_offset_h) << 16) | (blender.lcd_offset_w + blender.pipe[1].pipe_offset_w);
	
	de_commit();
}

void de_pipe(uint8_t pipe_0_enable, uint8_t pipe_1_enable)
{
	DE_MUX_GLB->CTL = ~BV(0); //disable
	DE_MUX_BLD->FILLCOLOR_CTL = (pipe_1_enable << 9) | // 0x0303 enable pipe 0,1 and pipe 0,1 fill color
								(pipe_0_enable << 8) | 
								(pipe_1_enable << 1) | 
								(pipe_0_enable << 0); 
	DE_MUX_GLB->CTL = BV(0); //enable
}

static void de_commit_wait(void)
{
	while (DE_MUX_GLB->DBUFFER & 1);
}

void de_commit(void)
{
	DE_MUX_GLB->DBUFFER = 1;
}

void de_layer_set(void *fb0, void *fb1, void *fb2)
{
	de_commit_wait();

	layers.layer[0].fb[0] = fb0;
	layers.layer[1].fb[0] = fb1;
	layers.layer[2].fb[0] = fb2;

/* 	layers.layer[0].fb[1] = fb1;
	layers.layer[0].fb_idx = 0;
	layers.layer[0].fb_dbl = fb1 != 0; */

	LOG_D("de: set layer, fmt = %d", layers.layer[0].fmt);
	uint32_t w = layers.layer[0].w;
	uint32_t h = layers.layer[0].h;
	uint32_t p = fmt_to_pitch(layers.layer[0].fmt);

	DE_MUX_OVL_V->LAYER[0].ATTCTL = BV(0) | (layers.layer[0].fmt << 8) | BV(15);
	DE_MUX_OVL_V->LAYER[0].MBSIZE = ((layers.layer[0].h-1) << 16) | (layers.layer[0].w-1); //layers heigh width
	DE_MUX_OVL_V->LAYER[0].COOR = ((layers.layer[0].offset_h) << 16) | (layers.layer[0].offset_w); //coor layer on overlay window
	DE_MUX_OVL_V->LAYER[0].PITCH0 = p * layers.layer[0].w; //layer memmory
	DE_MUX_OVL_V->LAYER[0].TOP_LADD0 = (uint32_t)layers.layer[0].fb[0];
	DE_MUX_OVL_V->SIZE = ((layers.layer[0].h-1) << 16) | (layers.layer[0].w-1); //overlay heigh width

	DE_MUX_OVL_V->LAYER[1].ATTCTL = BV(0) | (layers.layer[2].fmt << 8) | BV(15);
	DE_MUX_OVL_V->LAYER[1].MBSIZE = ((layers.layer[2].h-1) << 16) | (layers.layer[2].w-1); //layers heigh width
	DE_MUX_OVL_V->LAYER[1].COOR = ((layers.layer[2].offset_h) << 16) | (layers.layer[2].offset_w); //coor layer on overlay window
	DE_MUX_OVL_V->LAYER[1].PITCH0 = p * layers.layer[2].w; //layer memmory
	DE_MUX_OVL_V->LAYER[1].TOP_LADD0 = (uint32_t)layers.layer[2].fb[0];

 	DE_MUX_OVL_UI1->LAYER[0].ATTCTL = BV(0) | (layers.layer[1].fmt << 8) | BV(1) | (layers.layer[1].alpha << 24); 
	DE_MUX_OVL_UI1->LAYER[0].MBSIZE = ((layers.layer[1].h-1) << 16) | (layers.layer[1].w-1); //layers heigh width
	DE_MUX_OVL_UI1->LAYER[0].COOR = ((layers.layer[1].offset_h) << 16) | (layers.layer[1].offset_w); //coor layer on overlay window
	DE_MUX_OVL_UI1->LAYER[0].PITCH = p * layers.layer[1].w; //layer memmory
	DE_MUX_OVL_UI1->LAYER[0].TOP_LADD = (uint32_t)layers.layer[1].fb[0];
	DE_MUX_OVL_UI1->SIZE = ((layers.layer[1].h-1) << 16) | (layers.layer[1].w-1); //overlay heigh width 

	uint32_t src_w = layers.layer[1].w;
	uint32_t src_h = layers.layer[1].h;
	uint32_t dst_w = blender.pipe[1].pipe_w;
	uint32_t dst_h = blender.pipe[1].pipe_h;

#if 1
		uint64_t tmp = 0;
		uint64_t vstep = 0;
		// enable GSU (scaler unit)
		LOG_D("de: enable scaler");

		// set input resolution and output resolution
		DE_MUX_GSU1->OUTSIZE_REG = ((dst_h-1) << 16) | (dst_w - 1);
		DE_MUX_GSU1->INSIZE_REG = ((src_h-1) << 16) | (src_w - 1);

		// calculate fractional hstep
		tmp = src_w << GSU_PHASE_FRAC_BITWIDTH;
		tmp = tmp / dst_w;

		DE_MUX_GSU1->HSTEP_REG = tmp << GSU_PHASE_FRAC_REG_SHIFT;

		// calculate fractional vstep
		vstep = src_h << GSU_PHASE_FRAC_BITWIDTH;
		vstep = vstep / dst_h;

		DE_MUX_GSU1->VSTEP_REG = vstep << GSU_PHASE_FRAC_REG_SHIFT;

		// calculate hphase (its always zero for our purpose)
		tmp = (0 & 0xFFFFFFFF) >> (32 - GSU_PHASE_FRAC_BITWIDTH);
		DE_MUX_GSU1->HPHASE_REG = tmp << GSU_PHASE_FRAC_REG_SHIFT;

		// calculate vphase (its always zero for our purpose)
		tmp = (0 & 0xFFFFFFFF) >> (32 - GSU_PHASE_FRAC_BITWIDTH);
		DE_MUX_GSU1->VPHASE0_REG = tmp << GSU_PHASE_FRAC_REG_SHIFT;

		uint32_t pt_coef = 0;
		uint32_t scale_ratio, int_part, float_part, fir_coef_ofst;
		scale_ratio = vstep >> (GSU_PHASE_FRAC_BITWIDTH - 3);
		int_part = scale_ratio >> 3;
		float_part = scale_ratio & 0x7;
		fir_coef_ofst = (int_part == 0) ? GSU_ZOOM0_SIZE :
				(int_part == 1) ? GSU_ZOOM0_SIZE + float_part :
				(int_part == 2) ? GSU_ZOOM0_SIZE + GSU_ZOOM1_SIZE + (float_part >> 1) :
				(int_part == 3) ? GSU_ZOOM0_SIZE + GSU_ZOOM1_SIZE	+ GSU_ZOOM2_SIZE :
				(int_part == 4) ? GSU_ZOOM0_SIZE + GSU_ZOOM1_SIZE	+ GSU_ZOOM2_SIZE + GSU_ZOOM3_SIZE :
				GSU_ZOOM0_SIZE + GSU_ZOOM1_SIZE + GSU_ZOOM2_SIZE	+ GSU_ZOOM3_SIZE + GSU_ZOOM4_SIZE;

		pt_coef = fir_coef_ofst * GSU_PHASE_NUM;

		// copy fir table
		for (size_t i = 0; i < GSU_PHASE_NUM; i++) {
			DE_MUX_GSU1->HCOEF_REG[i] = lan2coefftab16[pt_coef + i];
		}
		DE_MUX_GSU1->CTRL_REG = 1 | 0x10;
#endif
	LOG_D("de: commiting");
	de_commit();
}

uint32_t de_layer_get_h(void)
{
	return layers.layer[0].h;
}

uint32_t de_layer_get_w(void)
{
	return layers.layer[0].w;
}

void *de_layer_get_fb(void)
{
	uint8_t idx = layers.layer[0].fb_idx;

	return layers.layer[0].fb[idx];
}

int de_layer_swap_done(void)
{
	if (layers.layer[0].fb_dbl == 0)
		return 1;

	uint32_t sp;

	//!portENTER_CRITICAL();
	sp = layers.layer[0].swap_pending == 0;
	//!portEXIT_CRITICAL();

	return sp;
}

void de_layer_swap(void)
{
	//!portENTER_CRITICAL();
	layers.layer[0].swap_pending = 1;
	//!portEXIT_CRITICAL();
}

void de_layer_register_semaphore() //! SemaphoreHandle_t s)
{
	//layers.layer[0].semaphore = s;
}

void de_int_vblank(void)
{
	uint32_t changed = 0;

	for (uint32_t i = 0; i < ARRAY_SIZE(layers.layer); i++) {
		if (layers.layer[i].fb_dbl == 0) continue;

		if (layers.layer[i].swap_pending) {
			uint8_t idx = layers.layer[i].fb_idx;

			DE_MUX_OVL_UI1->LAYER[i].TOP_LADD = (uint32_t)layers.layer[0].fb[idx];

			idx = !idx;

			layers.layer[i].fb_idx = idx;
			layers.layer[i].swap_pending = 0;
			changed = 1;
		}

/*		if (layers.layer[i].semaphore != NULL) {
			BaseType_t xHigherPriorityTaskWoken;
			xSemaphoreGiveFromISR(layers.layer[i].semaphore, &xHigherPriorityTaskWoken);
		} */
	}

	if (changed) {
		de_commit();
	}
}
