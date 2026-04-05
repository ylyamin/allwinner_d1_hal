#include "doomkeys.h"
#include "doomgeneric.h"
#include "log.h"
#include "ccu.h"

unsigned long systick_cnt;

int mkdir(const char *pathname, size_t mode)
{
    small_printf("mkdir not implemented");
    while (1)
        ;
    return -1;
}

void DG_Init()
{
    small_printf("DG_Init\r\n");
}


void DG_DrawFrame()
{
    small_printf("DG_DrawFrame not implemented\r\n");
    // while (1);
}

void DG_SleepMs(uint32_t ms)
{
    delay_ms(ms);
}

uint32_t DG_GetTicksMs()
{
    return get_time_ms();
}

int up = 0, down = 0, left = 0, right = 0, enter = 0, fire = 0;

int DG_GetKey(int* pressed, unsigned char* doomKey)
{

        *pressed = up;
        *doomKey = KEY_UPARROW;
        return 1;

        *pressed = down;
        *doomKey = KEY_DOWNARROW;
        return 1;

        *pressed = left;
        *doomKey = KEY_LEFTARROW;
        return 1;

        *pressed = right;
        *doomKey = KEY_RIGHTARROW;
        return 1;

        *pressed = enter;
        *doomKey = KEY_ENTER;
        return 1;

        *pressed = fire;
        *doomKey = KEY_FIRE;
        return 1;

	return 0;
}

void DG_SetWindowTitle(const char * title)
{
    small_printf("DG_SetWindowTitle not implemented\n");
}