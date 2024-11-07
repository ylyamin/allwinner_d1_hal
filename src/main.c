#include <main.h>

	#define GPIO_BASE (0x02000000)
	#define GPIO_OFF_PC_CFG (GPIO_BASE + 0x60)
	#define GPIO_OFF_PC_DAT (GPIO_BASE + 0x70)
	#define writeu32(addr, val) (*(uint32_t *)(addr) = (val))

void main(void)
{
	//csr_write_mie(1);
	//ccu_init();
	extern void dly(unsigned long us);
	GPIOC->CFG[0] = (0x1 << 4);
	GPIOC->DATA = (1 << 1);
	//dly(20000);
	//GPIOC->DATA = (0 << 1);

	//led_init();
	//led_set(0, 1);



	//writeu32(GPIO_OFF_PC_CFG, 0xFFFFFF1F); // set PC_1 to OUTPUT mode
	//writeu32(GPIO_OFF_PC_DAT, 0x00000002); // set PC_1 state to 1

	//uart_init(115200);
	//small_printf("\n\nHello from allwinner\n");
}