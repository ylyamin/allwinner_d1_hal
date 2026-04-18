#ifndef GR_h_
#define GR_h_

void gr_fill(void *fb, uint32_t w, uint32_t h, uint32_t c);
uint32_t gr_get_pixel(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y);
void gr_draw_pixel(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y, uint32_t c);
void gr_draw_circle(void *fb, uint32_t w, uint32_t h, int16_t xx, int16_t yy, int16_t r, uint32_t color);
void gr_draw_vline_xyh(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y, int16_t hh, uint32_t color);
void gr_draw_vline_xyy(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y0, int16_t y1, uint32_t color);
void gr_draw_hline_xyw(void *fb, uint32_t w, uint32_t h, int16_t x, int16_t y, int16_t ww, uint32_t color);
void gr_draw_hline_xxy(void *fb, uint32_t w, uint32_t h, int16_t x0, int16_t x1, int16_t y, uint32_t color);
void gr_draw_line(void *fb, uint32_t w, uint32_t h, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color);

#endif
