#ifndef DE_H_
#define DE_H_
#include "de_priv.h"

struct blender_t {
	uint32_t lcd_w;
	uint32_t lcd_h;
	uint32_t lcd_offset_w;
	uint32_t lcd_offset_h;
	// pipes
	struct {
		uint32_t pipe_w;
		uint32_t pipe_h;
		uint32_t pipe_offset_w;
		uint32_t pipe_offset_h;
	} pipe[4];
};

struct layer_t {
	uint32_t lcd_w;
	uint32_t lcd_h;
	// fb info
	uint16_t w;
	uint16_t h;
	void *fb[2];
	uint32_t fb_idx;
	uint32_t fb_draw_idx;
	uint32_t fb_dbl;
	uint32_t swap_pending;
	uint32_t fmt;

	uint8_t alpha;

	//! SemaphoreHandle_t semaphore;

	// overlay window
	struct {
		uint32_t x0;
		uint32_t y0;
		uint32_t x1;
		uint32_t y1;
	} win;
};

void de_config(struct layer_t layer, struct blender_t new_blender);
void de_init(void);
void de_commit(void);
uint32_t fmt_to_pitch(uint8_t fmt);

void de_layer_set(void *fb, void *fb1);
uint32_t de_layer_get_h(void);
uint32_t de_layer_get_w(void);
void *de_layer_get_fb(void);
int de_layer_swap_done(void);
void de_layer_swap(void);
//void de_layer_register_semaphore(SemaphoreHandle_t s);

void de_int_vblank(void);

#endif /* DE_H_ */
