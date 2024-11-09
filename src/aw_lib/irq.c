#include <platform.h>
  
void irq_enable(int irq)
{
	small_printf("irq_enable %d\n\r", irq);
	PLIC->PLIC_IP_REGn[irq / 32] |= (1 << (irq % 32));
	PLIC->PLIC_PRIO_REGn[irq] = 1;
	PLIC->PLIC_MIE_REGn[irq / 32] |= (1 << (irq % 32));
}

void irq_disable(int irq)
{
	PLIC->PLIC_MIE_REGn[irq / 32] &= ~(1 << (irq % 32));
}

void handle_trap(void)
{
	uint64_t mcause = csr_read_mcause();
	//uint64_t mepc = csr_read_mepc();

    if (mcause >> 63) {
        handle_interrupt(mcause);
    } else {
        handle_exception(mcause);
/*         asm volatile("csrr t0, mepc");
        asm volatile("addi t0, t0, 0x4");
        asm volatile("csrw mepc, t0"); */
    }
}

void handle_exception(uint64_t mcause) {
    small_printf("Unknown exception: %x\n\r",mcause);
	while(1)
		;
}

void handle_interrupt(uint64_t mcause) {
	uint32_t irq = PLIC->PLIC_MCLAIM_REG;
	if(irq == USB1_OHCI_IRQn)
	{
		small_printf("USB1_OHCI_IRQn interrupt\n\r");
		usb_int_handler();
	}else{
		small_printf("Unknown interrupt: %d\n\r",irq);
		while(1)
			;
	}
	PLIC->PLIC_MCLAIM_REG = irq;
}

static void plic_init()
{
	int irq; 
	int irq_max = 223;
	for(irq = 0; irq < irq_max; irq++)
	{
		if (irq > 16)
		{
			PLIC->PLIC_IP_REGn[irq / 32] &= ~(1 << (irq % 32));
			PLIC->PLIC_PRIO_REGn[irq] = 0;
			PLIC->PLIC_MIE_REGn[irq / 32] &= ~(1 << (irq % 32));
		}
	}
}

void irq_init(void)
{
	plic_init();
	csr_set_bits_mie(MIE_MEI_BIT_MASK);	
	csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}
