#include <platform.h>
#include <platform.h>
#include <log.h>
#include <gr.h>
#include <de.h>
#include <tcon_lcd.h>
#include <ccu.h>
#include <dsi.h>
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
			.lcd_w = timing.lcd_w,
			.lcd_h = timing.lcd_h,
			.w = timing.scale_w,
			.h = timing.scale_h,
			.fmt = LAY_FBFMT_ARGB_8888,
			.alpha = 0xff,
			.win = {
				.x0 = 0,
				.y0 = 0,
				.x1 = timing.lcd_w,
				.y1 = timing.lcd_h,
			},
	};
 	de_set_layer(layer);

	uint32_t w = de_layer_get_w();
 	uint32_t h = de_layer_get_h();
    framebuffer = tlsf_malloc(mem_pool, w * h * 4);

	gr_fill(framebuffer,w,h, 0x00000000);
 	gr_draw_line(framebuffer,w,h, 0, 0, w-1, h-1, 0xff00ffff);
	gr_draw_line(framebuffer,w,h, w-1, 0, 0, h-1, 0xff00ffff);

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
/* 	uint32_t h = de_layer_get_h();
	uint32_t w = de_layer_get_w();
	unsigned long ms = get_time_ms();

    if (!(ms % 200))
	{
		gr_draw_line(&framebuffer, w, h, line_x, 0, line_x, h-1, 0xff0000ff);	// clean previous
		gr_draw_line(&framebuffer, w, h, 0, line_y, w-1, line_y, 0xff0000ff);	// clean previous

		line_x += w / 20;
		line_y += h / 20;

		gr_draw_line(&framebuffer, w, h, line_x, 0, line_x, h-1, 0xff00ff00);	// draw new
		gr_draw_line(&framebuffer, w, h, 0, line_y, w-1, line_y, 0xffff0000);	// draw new

		if (line_x > w) line_x = 0; 									//reset in the end
		if (line_y > h) line_y = 0; 									//reset in the end
	}	   */
  
}