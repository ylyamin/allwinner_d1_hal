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

struct layer_t layers[1] = {
	{
		.lcd_w = 600,
		.lcd_h = 1280,
		.w = 600, //320,
		.h = 1280, //200,
		.fmt = LAY_FBFMT_ARGB_8888,
		.alpha = 0xff,

		.win = {
			.x0 = 0,
			.y0 = 0,
			.x1 = 600,
			.y1 = 1280,
		},
	},
};


uint32_t fmt_to_pitch(uint8_t fmt)
{
	return fmtpitch[fmt];
}

void de_set_layer(struct layer_t layer)
{
	layers[0] = layer;
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
	DE_MUX_GLB->SIZE = ((layers[0].lcd_h-1) << 16) | (layers[0].lcd_w-1);

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
	DE_MUX_BLD->FILLCOLOR_CTL = 0x0101; // enable pipe 0 and pipe 0 fill color
	DE_MUX_BLD->CH_RTCTL = 0x1; // route channel 1(UI1) to pipe 0 of blender
	DE_MUX_BLD->PREMUL_CTL = 0; //all alpha data is no-pre-multiply alpha
	DE_MUX_BLD->BK_COLOR = 0x80FF00; // RGB no alpha
	DE_MUX_BLD->SIZE = ((layers[0].lcd_h-1) << 16) | (layers[0].lcd_w-1); // lcd size

	// no color keying 
	DE_MUX_BLD->KEY_CTL = 0;
	DE_MUX_BLD->OUT_COLOR = 0;

	DE_MUX_BLD->CTL[0] = 0x03010301;
	DE_MUX_BLD->PIPE[0].FILL_COLOR = 0xFFFF8000;
	DE_MUX_BLD->PIPE[0].CH_ISIZE = ((768-1) << 16) | (480-1);
	DE_MUX_BLD->PIPE[0].CH_OFFSET = (256 << 16) | (60);

	de_commit();
}

static void de_commit_wait(void)
{
	while (DE_MUX_GLB->DBUFFER & 1);
}

void de_commit(void)
{
	DE_MUX_GLB->DBUFFER = 1;
}

#define SUN8I_UI_SCALER_SCALE_MIN		1
#define SUN8I_UI_SCALER_SCALE_MAX		((1UL << 20) - 1)

#define SUN8I_UI_SCALER_SCALE_FRAC		20
#define SUN8I_UI_SCALER_PHASE_FRAC		20
#define SUN8I_UI_SCALER_COEFF_COUNT		16

#define DE2_VI_SCALER_UNIT_BASE 0x20000
#define DE2_VI_SCALER_UNIT_SIZE 0x20000
#define DE2_UI_SCALER_UNIT_SIZE 0x10000

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

void de_layer_set(void *fb0, void *fb1)
{
	de_commit_wait();

	layers[0].fb[0] = fb0;
	layers[0].fb[1] = fb1;
	layers[0].fb_idx = 0;
	layers[0].fb_dbl = fb1 != 0;

	LOG_D("de: set layer, fmt = %d", layers[0].fmt);
	uint32_t w = layers[0].w;
	uint32_t h = layers[0].h;
	uint32_t p = fmt_to_pitch(layers[0].fmt);

	DE_MUX_OVL_UI1->LAYER[0].ATTCTL = BV(0) | (layers[0].fmt << 8) | BV(1) | (layers[0].alpha << 24); 
	DE_MUX_OVL_UI1->LAYER[0].MBSIZE = ((layers[0].h-1) << 16) | (layers[0].w-1); //layers heigh width
	DE_MUX_OVL_UI1->LAYER[0].COOR = ((layers[0].win.y0) << 16) | (layers[0].win.x0); //coor layer on overlay window
	DE_MUX_OVL_UI1->LAYER[0].PITCH = p * layers[0].w; //layer memmory
	DE_MUX_OVL_UI1->LAYER[0].TOP_LADD = (uint32_t)layers[0].fb[0];

	//DE_MUX_OVL_UI1->SIZE = ((layers[0].lcd_h-1) << 16) | (layers[0].lcd_w-1); //overlay lcd heigh lcd width
	DE_MUX_OVL_UI1->SIZE = ((320-1) << 16) | (200-1); //overlay lcd heigh lcd width

	unsigned int src_w = 200;
	unsigned int src_h = 320;

	unsigned int dst_w = 480;
	unsigned int dst_h = 768;

#if 1
	uint32_t hscale = (0x10000 << 4) * src_w / dst_w;
	uint32_t vscale = (0x10000 << 4) * src_h / dst_h;
	uint32_t layer = 0;

	uint32_t base, hphase = 0, vphase = 0;
	int i, offset;

	base = DE2_VI_SCALER_UNIT_BASE +
		DE2_VI_SCALER_UNIT_SIZE * 1 +
		DE2_UI_SCALER_UNIT_SIZE * layer;

	hphase <<= SUN8I_UI_SCALER_PHASE_FRAC - 16;
	vphase <<= SUN8I_UI_SCALER_PHASE_FRAC - 16;

	DE_MUX_GSU1->INSIZE_REG  = ((src_h-1) << 16) | (src_w - 1);
	DE_MUX_GSU1->OUTSIZE_REG = ((dst_h-1) << 16) | (dst_w - 1);
	DE_MUX_GSU1->HSTEP_REG = hscale;
	DE_MUX_GSU1->VSTEP_REG = vscale;
	DE_MUX_GSU1->HPHASE_REG = hphase;
	DE_MUX_GSU1->VPHASE0_REG = vphase;

	offset = sun8i_ui_scaler_coef_index(hscale) *
			SUN8I_UI_SCALER_COEFF_COUNT;
	for (i = 0; i < SUN8I_UI_SCALER_COEFF_COUNT; i++)
		DE_MUX_GSU1->HCOEF_REG[i] = lan2coefftab16[offset + i];

	DE_MUX_GSU1->CTRL_REG = 1 | 0x10;

#endif

#if 0
//	if  ((w < layers[0].lcd_w) || (h < layers[0].lcd_h)) {
		unsigned long long tmp = 0;
		unsigned long long vstep = 0;

		// enable GSU (scaler unit)
		LOG_D("de: enable scaler");

		// set input resolution and output resolution
		DE_MUX_GSU1->OUTSIZE_REG = ((dst_h-1) << 16) | (dst_w - 1);
		DE_MUX_GSU1->INSIZE_REG = ((src_h-1) << 16) | (src_w - 1);

		// calculate fractional hstep
		tmp = src_w << GSU_PHASE_FRAC_BITWIDTH;
		tmp = tmp / dst_w;
		//tmp = (unsigned int)(tmp >> GSU_FB_FRAC_BITWIDTH);

		DE_MUX_GSU1->HSTEP_REG = tmp << GSU_PHASE_FRAC_REG_SHIFT;
		LOG_D("de: hstep = %08x\n\r", DE_MUX_GSU1->HSTEP_REG);
		LOG_D("de: hstep = %08x\n\r", (0x10000 << 4) * src_w / dst_w);

		// calculate fractional vstep
		vstep = src_h << GSU_PHASE_FRAC_BITWIDTH;
		vstep = vstep / dst_h;
		//vstep = (unsigned int)(vstep >> GSU_FB_FRAC_BITWIDTH);

		DE_MUX_GSU1->VSTEP_REG = vstep << GSU_PHASE_FRAC_REG_SHIFT;
		LOG_D("de: vstep = %08x\n\r", DE_MUX_GSU1->VSTEP_REG);
		LOG_D("de: vstep = %08x\n\r", (0x10000 << 4) * src_h / dst_h);


		// calculate hphase (its always zero for our purpose)
		tmp = (0 & 0xFFFFFFFF) >> (32 - GSU_PHASE_FRAC_BITWIDTH);
		DE_MUX_GSU1->HPHASE_REG = tmp << GSU_PHASE_FRAC_REG_SHIFT;
//		LOG_D("de: hphase = %08x\n\r", DE_MUX_GSU1->HPHASE_REG);

		// calculate vphase (its always zero for our purpose)
		tmp = (0 & 0xFFFFFFFF) >> (32 - GSU_PHASE_FRAC_BITWIDTH);
		DE_MUX_GSU1->VPHASE0_REG = tmp << GSU_PHASE_FRAC_REG_SHIFT;
//		LOG_D("de: vphase = %08x\n\r", DE_MUX_GSU1->VPHASE0_REG);

		unsigned int pt_coef = 0;
		
		{
			unsigned int scale_ratio, int_part, float_part, fir_coef_ofst;
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
		}

//		LOG_D("de: pt_coef = %d\n\r", pt_coef);

		// copy fir table
		for (size_t i = 0; i < GSU_PHASE_NUM; i++) {
			DE_MUX_GSU1->HCOEF_REG[i] = lan2coefftab16[pt_coef + i];
		}

		DE_MUX_GSU1->CTRL_REG = 1 | 0x10;
	//}
#endif

	LOG_D("de: commiting");
	de_commit();
}

uint32_t de_layer_get_h(void)
{
	return layers[0].h;
}

uint32_t de_layer_get_w(void)
{
	return layers[0].w;
}

void *de_layer_get_fb(void)
{
	uint8_t idx = layers[0].fb_idx;

	return layers[0].fb[idx];
}

int de_layer_swap_done(void)
{
	if (layers[0].fb_dbl == 0)
		return 1;

	uint32_t sp;

	//!portENTER_CRITICAL();
	sp = layers[0].swap_pending == 0;
	//!portEXIT_CRITICAL();

	return sp;
}

void de_layer_swap(void)
{
	//!portENTER_CRITICAL();
	layers[0].swap_pending = 1;
	//!portEXIT_CRITICAL();
}

void de_layer_register_semaphore() //! SemaphoreHandle_t s)
{
	//layers[0].semaphore = s;
}

void de_int_vblank(void)
{
	uint32_t changed = 0;

	for (uint32_t i = 0; i < ARRAY_SIZE(layers); i++) {
		if (layers[i].fb_dbl == 0) continue;

		if (layers[i].swap_pending) {
			uint8_t idx = layers[i].fb_idx;

			DE_MUX_OVL_UI1->LAYER[i].TOP_LADD = (uint32_t)layers[0].fb[idx];

			idx = !idx;

			layers[i].fb_idx = idx;
			layers[i].swap_pending = 0;
			changed = 1;
		}

/*		if (layers[i].semaphore != NULL) {
			BaseType_t xHigherPriorityTaskWoken;
			xSemaphoreGiveFromISR(layers[i].semaphore, &xHigherPriorityTaskWoken);
		} */
	}

	//if (changed) {
		de_commit();
	//}
}
