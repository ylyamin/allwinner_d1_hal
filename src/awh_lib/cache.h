#ifndef __RISCV64_CACHE_H__
#define __RISCV64_CACHE_H__

#ifdef __cplusplus
extern "C" {
#endif

void dcache_wb_range(unsigned long start, unsigned long end);
void dcache_inv_range(unsigned long start, unsigned long end);
void dcache_wbinv_range(unsigned long start, unsigned long end);

#ifdef __cplusplus
}
#endif

#endif /* __RISCV64_CACHE_H__ */
