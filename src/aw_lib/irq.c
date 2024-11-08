/*
 * driver/irq-d1.c
 *
 * Copyright(c) 2007-2023 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <platform.h>

void irq_enable(int irq)
{
	small_printf("irq_enable %d\n\r",irq);

	PLIC->PLIC_MIE_REGn[irq >> 5] |= (1 << (irq & 0x1f));
	
	uint32_t val = PLIC->PLIC_MIE_REGn[irq >> 5];

	small_printf("PLIC_MIE_REGn %x\n\r",val);
}

void irq_disable(int irq)
{
	PLIC->PLIC_MIE_REGn[irq >> 5] &= ~(1 << (irq & 0x1f));
}

void handle_trap(void)
{
	small_printf("Handle_trap\n\r");

	uint64_t mcause, mepc;
    asm volatile("csrr %0, mcause" : "=r"(mcause));
    asm volatile("csrr %0, mepc" : "=r"(mepc));

    small_printf("mcause: %x",mcause);

    if (mcause >> 63) {
        handle_interrupt(mcause);
    } else {
        handle_exception(mcause);
        asm volatile("csrr t0, mepc");
        asm volatile("addi t0, t0, 0x4");
        asm volatile("csrw mepc, t0");
    }
}

void handle_exception(uint64_t mcause) {
    small_printf("Unknown exception: %x",mcause);
}

void handle_interrupt(uint64_t mcause) {
	uint32_t irq = PLIC->PLIC_MCLAIM_REG;
	if(irq == USB1_OHCI_IRQn)
	{
		small_printf("USB1_OHCI_IRQn interrupt");
		usb_int_handler();
	}else{
		small_printf("Unknown interrupt: %d",irq);
	}
	PLIC->PLIC_MCLAIM_REG = irq;
}

static void plic_init()
{
	int i; 
	int irq_max = 223;
	/*
	 * Disable all interrupts
	 */
	for(i = 0; i < irq_max; i++)
	{
		PLIC->PLIC_MIE_REGn[i >> 5] &= ~(1 << (i & 0x1f));
	}
	/*
	 * Set all source priorities to one
	 */
	for(i = 0; i < irq_max; i++)
	{
		PLIC->PLIC_PRIO_REGn[i] = 1;
	}
	/*
	 * Clear pending bits
	 */
	for(i = 0; i < irq_max; i++)
	{
		PLIC->PLIC_MCLAIM_REG = i;
	}
}

void irq_init(void)
{
	plic_init();
	csr_set_bits_mie(MIE_MEI_BIT_MASK);
	csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}
