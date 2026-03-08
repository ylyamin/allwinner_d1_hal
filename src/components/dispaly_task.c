#include <platform.h>
#include <platform.h>
#include <log.h>
#include <gr.h>
#include <de.h>
#include <tcon_lcd.h>
#include <ccu.h>
#include <dsi.h>

extern timing_t LCD_get_param(void);
extern void LCD_gpio_init(void);
extern void LCD_bl_open(void);
extern void LCD_bl_close(void);
extern void LCD_panel_init(void);

uint8_t fb1[1280 * 480 * 4];  //= dma_memalign(128, size);
uint8_t fb2[1280 * 480 * 4]; // = dma_memalign(128, size);

void display_task_init(void)
{
	LOG_D("display_task_init");

	timing_t timing = LCD_get_param();

	struct layer_t layer = {
			.lcd_w = 480,
			.lcd_h = 1280,
			.w = 480,
			.h = 1280,
			.fmt = LAY_FBFMT_RGBA_8888, 
			.alpha = 0xff,
			.win = {
				.x0 = 0,
				.y0 = 0,
				.x1 = 480,
				.y1 = 1280,
			},
	};

 	de_set_layer(layer);

	uint32_t w = 480; // de_layer_get_w();
	uint32_t h = 1280; // de_layer_get_h();

	gr_fill(&fb1,w,h, 0xff0000ff);
	gr_fill(&fb2,w,h, 0xff0000ff);

	gr_fill(&fb1, 400, 400, 0xff00ff00);

/* 	gr_draw_pixel(&fb1,w,h, 100, 100, 0xffff00ff);
	gr_draw_pixel(&fb1,w,h, 101, 101, 0xffff00ff);
	gr_draw_pixel(&fb1,w,h, 101, 100, 0xffff0000);
	gr_draw_pixel(&fb1,w,h, 100, 101, 0xffff0000); */
 	//gr_draw_line(&fb1,w,h, 0, 0, w-1, h-1, 0xff00ffff);
	//gr_draw_line(&fb1,w,h, w-1, 0, 0, h-1, 0xffff00ff);

	//1
	LCD_gpio_init();
	LCD_bl_open();

	//2
	tcon_lcd_init(timing);

	//3
	LCD_panel_init();

	//4
	tcon_lcd_enable();
	delay_ms(120); //120 

	de_init();
	de_layer_set(&fb1, &fb2);

	//tcon_dump_regs();
}

uint32_t line_x = 0;
uint32_t line_y = 0;

void display_task_exec(void)
{
	uint32_t h = de_layer_get_h();
	uint32_t w = de_layer_get_w();
	unsigned long ms = get_time_ms();




/*    	if (!(ms % 200))
	{
		gr_draw_line(&fb1, w, h, line_x, 0, line_x, h-1, 0xff00ffff);	// clean previous
		gr_draw_line(&fb1, w, h, 0, line_y, w-1, line_y, 0xff00ffff);	// clean previous

		line_x += w / 20;
		line_y += h / 20;

		gr_draw_line(&fb1, w, h, line_x, 0, line_x, h-1, 0xff00ff00);	// draw new
		gr_draw_line(&fb1, w, h, 0, line_y, w-1, line_y, 0xffff0000);	// draw new

		if (line_x > w) line_x = 0; 									//reset in the end
		if (line_y > h) line_y = 0; 									//reset in the end
	}	  
  */
}