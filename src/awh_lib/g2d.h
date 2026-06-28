#ifndef G2D_H_
#define G2D_H_
#include "platform.h"

typedef enum
{
    CW_0    = 0x0,
    CW_90   = 0x1,
    CW_180  = 0x2,
    CW_270  = 0x3,
} g2d_rot_angle_t;

struct g2d_rot_t {
    void *src_fb;
    void *dst_fb;
    uint32_t src_w;
	uint32_t src_h;
    uint32_t dst_w;
	uint32_t dst_h;
    g2d_rot_angle_t rot_angle;
    uint8_t vflip;
    uint8_t hflip;
    uint32_t fmt;
};

void g2d_init(void);
void g2d_rot(struct g2d_rot_t);
int g2d_rot_finish(void);

#endif /* G2D_H_ */