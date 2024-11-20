#include <platform.h>
#include <platform.h>
#include <log.h>
#include <gr.h>
#include <de.h>
#include <tcon_lcd.h>
#include <icn9707_480x1280.h>

void task_display(void)
{
	timing_t timing = LCD_get_param();

	struct layer_t layer = {
			.lcd_w = timing.lcd_w,
			.lcd_h = timing.lcd_h,
			.w = timing.lcd_w,
			.h = timing.lcd_h,
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

	uint32_t h = de_layer_get_h();
	uint32_t w = de_layer_get_w();

	uint32_t size = h * w * 4;
	uint8_t fb1[size];  //= dma_memalign(128, size);
	uint8_t fb2[size]; // = dma_memalign(128, size);

	gr_fill(&fb1,w,h, 0xff0000ff);
	gr_fill(&fb2,w,h, 0xff0000ff);
	gr_draw_pixel(&fb1,w,h, 100, 100, 0xffff0000);
	gr_draw_pixel(&fb1,w,h, 101, 101, 0xffff0000);
	gr_draw_pixel(&fb1,w,h, 101, 100, 0xffff0000);
	gr_draw_pixel(&fb1,w,h, 100, 101, 0xffff0000);
	gr_draw_line(&fb1,w,h, 0, 0, w-1, h-1, 0xff00ff00);
	gr_draw_line(&fb1,w,h, w-1, 0, 0, h-1, 0xffff0000);

	LCD_gpio_init();

	tcon_lcd_init(timing);
	tcon_lcd_enable();
	
	LCD_bl_open();
	LCD_panel_init();

	de_init();
	de_layer_set(&fb1, &fb2);
	
}