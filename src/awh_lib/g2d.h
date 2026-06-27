#ifndef G2D_H_
#define G2D_H_

struct g2d_rot_t {
    void *src_fb;
    void *dst_fb;
    uint16_t src_w;
	uint16_t src_h;
    uint16_t dst_w;
	uint16_t dst_h;
    uint32_t fmt;
};

void g2d_init(void);
void g2d_rot(struct g2d_rot_t);

#endif /* G2D_H_ */