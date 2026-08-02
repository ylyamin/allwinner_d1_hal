#include <platform.h>
#include <platform.h>
#include <log.h>
#include <gr.h>
#include <de.h>
#include <g2d.h>
#include <tcon_lcd.h>
#include <ccu.h>
#include <mipi_dsi.h>
#include <tlsf.h>

extern timing_t LCD_get_param(void);
extern void LCD_gpio_init(void);
extern void LCD_bl_open(void);
extern void LCD_bl_close(void);
extern void LCD_panel_init(void);

extern tlsf_t mem_pool;
extern uint8_t *console_framebuffer; 
extern uint8_t *gui_framebuffer; 
extern uint32_t *doom_framebuffer;

uint8_t *framebuffer1; 
uint8_t *framebuffer2; 
uint8_t *framebuffer3; 

struct g2d_rot_t g2d_rot_config;
struct g2d_rot_t g2d_rot_doom_config;
struct g2d_rot_t g2d_rot_config_gui;

#define BG_COLOR_1 0xff00004f
#define BG_COLOR_2 0xff004f00
#define LINE_COLOR_1 0xff4f4f4f

void display_task_init(void)
{
	LOG_D("display_task_init");
	timing_t timing = LCD_get_param();

	uint32_t w, h;
	uint32_t doom_w = 200;
	uint32_t doom_h = 320;
	uint32_t scaled_doom_w = timing.lcd_scale_w; //480
	uint32_t scaled_doom_h = timing.lcd_scale_w * doom_h / doom_w; //768

	struct blender_t blender = {
		.lcd_w = timing.lcd_w, //600
		.lcd_h = timing.lcd_h, //1280
		.lcd_offset_w = (timing.lcd_w - timing.lcd_scale_w)/2, //60
		.lcd_offset_h = (timing.lcd_h - timing.lcd_scale_h)/2, //0
		.pipe = { 
			{ //main screen
				.pipe_enable = 1,
				.pipe_w = timing.lcd_scale_w, //480
				.pipe_h = timing.lcd_scale_h, //1280
				.pipe_offset_w = 0,
				.pipe_offset_h = 0,
			},
			{ //doom screen
				.pipe_enable = 0,
				.pipe_w = scaled_doom_w, //480
				.pipe_h = scaled_doom_h, //768
				.pipe_offset_w = (timing.lcd_scale_w - scaled_doom_w) / 2, //0
				.pipe_offset_h = (timing.lcd_scale_h - scaled_doom_h) / 2, //256
			},
		},
	};

    framebuffer1 = tlsf_memalign(mem_pool, 16, timing.lcd_scale_w * timing.lcd_scale_h * 4);
    framebuffer2 = tlsf_memalign(mem_pool, 16, doom_w * doom_h * 4);
    framebuffer3 = tlsf_memalign(mem_pool, 16, timing.lcd_scale_h * 20 * 4);

	struct layer_t layers = {
		.layer = {
			{ //main screen
				.w = timing.lcd_scale_w,
				.h = timing.lcd_scale_h,
				.offset_w = 0,
				.offset_h = 0,
				.fmt = LAY_FBFMT_ARGB_8888,
				.alpha = 0xff,
			},	
			{ //doom screen
				.w = doom_w,
				.h = doom_h,
				.offset_w = 0,
				.offset_h = 0,
				.fmt = LAY_FBFMT_ARGB_8888, //    DE_FORMAT_8bpp_palette_LE = 0x1b,
				.alpha = 0xff,
			},	
			{ //gui screen
				.w = 20,
				.h = timing.lcd_scale_h,
				.offset_w = 0,
				.offset_h = 0,
				.fmt = LAY_FBFMT_ARGB_8888,
				.alpha = 0xff,
			},	
		}
	};

 	de_config(layers,blender);

//debug lines in framebuffer1 for pipe 0
	w = timing.lcd_scale_h;  //1280
 	h = timing.lcd_scale_w;  //480 
	gr_fill(framebuffer1,w,h, 0xFFFFFFFF);

#if 0
	gr_draw_hline_xyw(framebuffer1, w, h, /*x*/ 10,		/*y*/ 10,		/*ww*/ w - 20, LINE_COLOR_1);
	gr_draw_hline_xyw(framebuffer1, w, h, /*x*/ 10,		/*y*/ h - 10,	/*ww*/ w - 20, LINE_COLOR_1);
	gr_draw_vline_xyh(framebuffer1, w, h, /*x*/ 10,		/*y*/ 10,		/*hh*/ h - 20, LINE_COLOR_1);
	gr_draw_vline_xyh(framebuffer1, w, h, /*x*/ w - 10,  /*y*/ 10,		/*hh*/ h - 20, LINE_COLOR_1);
 	gr_draw_line(framebuffer1,w,h, 0, 0, w-1, h-1, LINE_COLOR_1);
	gr_draw_line(framebuffer1,w,h, w-1, 0, 0, h-1, LINE_COLOR_1);
#endif

//debug lines in framebuffer3 for pipe 1
	w = doom_w;
 	h = doom_h;

	gr_fill(framebuffer2,w,h, BG_COLOR_2);
 	gr_draw_line(framebuffer2,w,h, 0, 0, w-1, h-1, LINE_COLOR_1);
	gr_draw_line(framebuffer2,w,h, w-1, 0, 0, h-1, LINE_COLOR_1); 

//debug lines in framebuffer3 for pipe 0
	gr_fill(framebuffer3, w, 20, BG_COLOR_1);

	//1
	LCD_gpio_init();
	LCD_bl_open();

	//2
	tcon_lcd_init(timing);

	//3
	LCD_panel_init();
	
	//4
	tcon_lcd_enable();
	
	//5
	de_init();	
	g2d_init();

    g2d_rot_config.src_fb = console_framebuffer; //framebuffer;
    g2d_rot_config.dst_fb = framebuffer1;
    g2d_rot_config.src_w = timing.lcd_scale_h;//1280
    g2d_rot_config.src_h = timing.lcd_scale_w;//480
    g2d_rot_config.dst_w = timing.lcd_scale_w;//480
    g2d_rot_config.dst_h = timing.lcd_scale_h;//1280
	g2d_rot_config.rot_angle = CW_270;
    g2d_rot_config.fmt = LAY_FBFMT_ARGB_8888;

	g2d_rot(g2d_rot_config); //rotate framebuffer - after rotation vertical lines gone ?
	while(g2d_rot_finish());

    g2d_rot_doom_config.src_fb = doom_framebuffer;
    g2d_rot_doom_config.dst_fb = framebuffer2;
    g2d_rot_doom_config.src_w = 320;
    g2d_rot_doom_config.src_h = 200;
    g2d_rot_doom_config.dst_w = 200;
    g2d_rot_doom_config.dst_h = 320;
    g2d_rot_doom_config.rot_angle = CW_270;
    g2d_rot_doom_config.fmt = LAY_FBFMT_ARGB_8888;

    g2d_rot_config_gui.src_fb = gui_framebuffer;
    g2d_rot_config_gui.dst_fb = framebuffer3;
    g2d_rot_config_gui.src_w = timing.lcd_scale_h;//1280
    g2d_rot_config_gui.src_h = 20;//480
    g2d_rot_config_gui.dst_w = 20;//480
    g2d_rot_config_gui.dst_h = timing.lcd_scale_h;//1280
	g2d_rot_config_gui.rot_angle = CW_270;
    g2d_rot_config_gui.fmt = LAY_FBFMT_ARGB_8888;

   	g2d_rot(g2d_rot_config_gui); //rotate framebuffer - after rotation vertical lines gone ?
	while(g2d_rot_finish());

	de_layer_set(framebuffer1, framebuffer2, framebuffer3);

	//tcon_dump_regs();
}

uint32_t line_x = 0;
uint32_t line_y = 0;
unsigned long ms;

void display_task_exec(void)
{
#if 0
	//debug lines in framebuffer for pipe 0
 	uint32_t w = de_layer_get_h();
	uint32_t h = de_layer_get_w();
	timing_t timing = LCD_get_param();

    /* if (get_time_ms() > ms + 5)
	{
		ms = get_time_ms(); */

		gr_draw_line(framebuffer, w, h, line_x, 0, line_x, h-1, BG_COLOR_1);	// clean previous
		gr_draw_line(framebuffer, w, h, 0, line_y, w-1, line_y, BG_COLOR_1);	// clean previous

		gr_draw_line(framebuffer,w,h, 0, 0, w-1, h-1, LINE_COLOR_1);
		gr_draw_line(framebuffer,w,h, w-1, 0, 0, h-1, LINE_COLOR_1);

		line_x += w / 20;
		line_y += h / 20;

		gr_draw_line(framebuffer, w, h, line_x, 0, line_x, h-1, LINE_COLOR_1);	// draw new
		gr_draw_line(framebuffer, w, h, 0, line_y, w-1, line_y, LINE_COLOR_1);	// draw new

		if (line_x > w) line_x = 0; 									//reset in the end
		if (line_y > h) line_y = 0; 									//reset in the end
#endif
 		g2d_rot(g2d_rot_config); //rotate framebuffer
		while(!g2d_rot_finish());

		g2d_rot(g2d_rot_config_gui);
		while(g2d_rot_finish()); 

		de_commit();
	//} 

}