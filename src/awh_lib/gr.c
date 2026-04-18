#include "platform.h"
#include "gr.h"
#include <string.h>

void gr_fill(void *fb, uint32_t w, uint32_t h, uint32_t c)
{
	for (uint32_t y = 0; y < h; y++) {
		for (uint32_t x = 0; x < w; x++) {
			gr_draw_pixel(fb, w, h, x, y, c);
		}
	}
}

uint32_t gr_get_pixel(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y)
{
	if (x < 0 || y < 0 || x >= w || y >= h) return 0;

	return *(volatile uint32_t *)((uint32_t)fb + 4 * (y * w + x));
}

void gr_draw_pixel(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y, uint32_t c)
{
	if (x < 0 || y < 0 || x >= w || y >= h) return;
	*(volatile uint32_t *)((uint32_t)fb + 4 * (y * w + x)) = c;
}

void gr_draw_circle(void *fb, uint32_t w, uint32_t h, int16_t xx, int16_t yy, int16_t r, uint32_t color)
{ 
	int x, y, error;

	if (r == 0) {
		gr_draw_pixel(fb, w, h, xx, yy, color);
		return;
	}

	/* Clip negative r */
	r = r < 0 ? -r : r;

	for (x = 0, error = -r, y = r; y >= 0; y--) {
		/* Iterate X until we can pass to the next line. */
		while (error < 0) {
			error += 2*x + 1;
			x++;
			gr_draw_pixel(fb, w, h, xx-x+1, yy-y, color);
			gr_draw_pixel(fb, w, h, xx+x-1, yy-y, color);
			gr_draw_pixel(fb, w, h, xx-x+1, yy+y, color);
			gr_draw_pixel(fb, w, h, xx+x-1, yy+y, color);
		}

		/* Enough changes accumulated, go to next line. */
		error += -2*y + 1;
		gr_draw_pixel(fb, w, h, xx-x+1, yy-y, color);
		gr_draw_pixel(fb, w, h, xx+x-1, yy-y, color);
		gr_draw_pixel(fb, w, h, xx-x+1, yy+y, color);
		gr_draw_pixel(fb, w, h, xx+x-1, yy+y, color);
	}
}

void gr_draw_vline_xyh(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y, int16_t hh, uint32_t color)
{
	if ((x >= w) || (y >= h)) return;
	if ((y+h-1) >= h) hh = w - y;

	for (int i = 0; i < hh; i++) {
		gr_draw_pixel(fb, w, hh, x, y+i, color);
	}
}

void gr_draw_vline_xyy(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y0, int16_t y1, uint32_t color)
{
	int16_t hh;

	if ((x >= w) || (y0 >= h)) return;
	if ((y1) >= h) y1 = h-1;

	hh = MAX(y0, y1) - MIN(y0, y1);

	gr_draw_vline_xyh(fb, w, h, x, MIN(y0, y1), hh, color);
}

void gr_draw_hline_xyw(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y, int16_t ww, uint32_t color)
{
	if ((x >= w) || (y >= h)) return;
	if ((x + ww - 1) >= w)  ww = w - x;

	for (int i = 0; i < ww; i++) {
		gr_draw_pixel(fb, w, h, x+i, y, color);
	}
}

void gr_draw_hline_xxy(void *fb, uint32_t w, uint32_t h, int16_t x0, int16_t x1, int16_t y, uint32_t color)
{
	int16_t ww;

	// Rudimentary clipping
	if ((x0 >= w) || (y >= h)) return;
	if (x1 >= w)  x1 = w-1;

	ww = MAX(x0, x1) - MIN(x0, x1) + 1;

	gr_draw_hline_xyw(fb, w, h, MIN(x0, x1), y, ww, color);
}


void gr_draw_line(void *fb, uint32_t w, uint32_t h, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)
{
	if (x0 == x1) {
		if (y0 == y1) {
			gr_draw_pixel(fb, w, h, x0, y0, color);
			return;
		}
		gr_draw_vline_xyy(fb, w, h, x0, y0, y1, color);
		return;
	}
	if (y0 == y1) {
		gr_draw_hline_xxy(fb, w, h, x0, x1, y0, color);
		return;
	}

	int steep = ABS(y1 - y0) / ABS(x1 - x0);
	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}

	int deltax = x1 - x0;
	int deltay = ABS(y1 - y0);

	int error = deltax / 2;

	int y = y0, x;
	int ystep = (y0 < y1) ? 1 : -1;
	for (x = x0; x <= x1; x++) {

		if (steep) {
			gr_draw_pixel(fb, w, h, y, x, color);
		} else {
			gr_draw_pixel(fb, w, h, x, y, color);
		}

		error -= deltay;
		if (error < 0) {
			y += ystep;
			error += deltax;
		}
	}
}
