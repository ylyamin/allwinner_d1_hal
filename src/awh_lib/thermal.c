#include <platform.h>
#define OFFSET              -2794
#define SCALE               -67

void ths_init(void)
{
	CCU->THS_BGR_REG |= BV(16);
	CCU->THS_BGR_REG |= BV(0);
    THS->THS_CTRL |= 0x2F;
    THS->THS_CTRL |= (0x1DF << 16);
    THS->THS_PER |= (0x3A << 12);
    THS->THS_FILTER = BV(2) | BV(0);
    THS->THS_EN = BV(0);
}

uint32_t ths_get_temp(void)
{
/*     if(THS->THS_DATA_INTS)
    {   
        THS->THS_DATA_INTS |= BV(0); */
        return (THS->THS_DATA + OFFSET) * SCALE / 1000;
/*     } */
}
