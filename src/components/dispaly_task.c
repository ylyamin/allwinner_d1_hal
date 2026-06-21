#include <platform.h>
#include <platform.h>
#include <log.h>
#include <gr.h>
#include <de.h>
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
unsigned char *framebuffer; 

void display_task_init(void)
{
	LOG_D("display_task_init");
	timing_t timing = LCD_get_param();
	struct layer_t layer = {
			.lcd_w = 600,
			.lcd_h = 1280,
			.w = 480,
			.h = 1280,
			.fmt = LAY_FBFMT_ARGB_8888,
			.alpha = 0xff,
			.win = {
				.x0 = 60,
				.y0 = 0,
				.x1 = 480 + 60,
				.y1 = 1280,
			},
	};
 	de_set_layer(layer);

	uint32_t w = de_layer_get_w();
 	uint32_t h = de_layer_get_h();
    framebuffer = tlsf_malloc(mem_pool, w * h * 4);
	gr_fill(framebuffer,w,h, 0x000000ff);

	gr_draw_hline_xyw(framebuffer, w, h, /*x*/ 10,		/*y*/ 10,		/*ww*/ w - 20, 0x00ff0000);
	gr_draw_hline_xyw(framebuffer, w, h, /*x*/ 10,		/*y*/ h - 10,	/*ww*/ w - 20, 0x0000ff00);
	gr_draw_vline_xyh(framebuffer, w, h, /*x*/ 10,		/*y*/ 10,		/*hh*/ h - 10, 0x00ff00ff);
	gr_draw_vline_xyh(framebuffer, w, h, /*x*/ w - 10,  /*y*/ 10,		/*hh*/ h - 10, 0x00ffff00);

 	//gr_draw_line(framebuffer,w,h, 0, 0, w-1, h-1, 0xff00ffff);
	//gr_draw_line(framebuffer,w,h, w-1, 0, 0, h-1, 0xff00ffff);

	//1
	LCD_gpio_init();
	LCD_bl_open();

	//2
	tcon_lcd_init(timing);

	//3
	LCD_panel_init();

	//4
	tcon_lcd_enable();

	de_init();
	de_layer_set(framebuffer, 0);

	//tcon_dump_regs();
}

uint32_t line_x = 0;
uint32_t line_y = 0;

void display_task_exec(void)
{
	uint32_t w = de_layer_get_w();
 	uint32_t h = de_layer_get_h();
	unsigned long ms = get_time_ms();

/*     if (!(ms % 200))
	{
		gr_draw_line(framebuffer, w, h, line_x, 0, line_x, h-1, 0x000000ff);	// clean previous
		gr_draw_line(framebuffer, w, h, 0, line_y, w-1, line_y, 0x000000ff);	// clean previous

		gr_draw_line(framebuffer,w,h, 0, 0, w-1, h-1, 0xff00ffff);
		gr_draw_line(framebuffer,w,h, w-1, 0, 0, h-1, 0xff00ffff);

		line_x += w / 20;
		line_y += h / 20;

		gr_draw_line(framebuffer, w, h, line_x, 0, line_x, h-1, 0xff00ffff);	// draw new
		gr_draw_line(framebuffer, w, h, 0, line_y, w-1, line_y, 0xff00ffff);	// draw new

		if (line_x > w) line_x = 0; 									//reset in the end
		if (line_y > h) line_y = 0; 									//reset in the end
	}	   */ 
   
}