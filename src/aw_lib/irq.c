/*
 * File: irq.c
 * Author: ylyamin
 */
#include <platform.h>

#define IRQ_NUM_MIN 16
#define IRQ_NUM_MAX 223

static const char * interrupt_names[] = {
	"User software interrupt",			//0
	"Supervisor software interrupt",	//1
	"Hypervisor software interrupt",	//2
	"Machine software interrupt",		//3
	"User timer interrupt",				//4
	"Supervisor timer interrupt",		//5
	"Hypervisor timer interrupt",		//6
	"Machine timer interrupt",			//7
	"User external interrupt",			//8
	"Supervisor external interrupt",	//9
	"Hypervisor external interrupt",	//10
	"Machine external interrupt",		//11
	"Reserved (12)",					//12
	"Reserved (13)",					//13
	"Reserved (14)",					//14
	"Reserved (15)",					//15
	"Reserved (16)",					//16
	"A performance monitoring overflo",	//17
};

static const char * exception_names[] = {
	"Instruction address misaligned",	//0
	"Instruction access fault",			//1
	"Illegal instruction",				//2
	"Breakpoint",						//3
	"Load address misaligned",			//4
	"Load access fault",				//5
	"Store address misaligned",			//6
	"Store access fault",				//7
	"Environment call from U-mode",		//8
	"Environment call from S-mode",		//9
	"Reserved (10)",					//10
	"Environment call from M-mode",		//11
	"Instruction page fault",			//12
	"Load page fault",					//13
	"Reserved (14)",					//14
	"Store page fault",					//15
};

void handle_trap(void)
{
	uint64_t mcause = csr_read_mcause();
	uint64_t mepc = csr_read_mepc();

    if (mcause >> 63) {
		if((mcause & ~(1UL << 63)) < ARRAY_SIZE(interrupt_names))
		{
			small_printf("Interrupt: %s\n\r", interrupt_names[mcause & ~(1UL << 63)]);
			handle_interrupt(mcause);
		}
		else
		{
			small_printf("Unknown interrupt %d\n\r", mcause & ~(1UL << 63));
			while(1)
				;
		}	
    } else {
		if(mcause < ARRAY_SIZE(exception_names))
		{
			small_printf("Exception: %s\r\n", exception_names[mcause]);
			handle_exception(mcause);
			asm volatile("csrr t0, mepc");
			asm volatile("addi t0, t0, 0x4");
			asm volatile("csrw mepc, t0");
		}
		else
		{
			small_printf("Unknown exception %d\r\n", mcause);
			while(1)
				;
		}	
    }
}

void handle_exception(uint64_t mcause) {
    small_printf("Handle exception not implemented");
	while(1)
		;
}

void handle_interrupt(uint64_t mcause) {
	// An external HW interrupt in M-mode
	if((mcause & ~(1UL << 63)) == 11)
	{
		//csr_clr_bits_mip(csr_read_mip() & (mcause & ~(1UL << 63)));
		uint32_t irq = PLIC->PLIC_MCLAIM_REG;
		if(irq == USB1_OHCI_IRQn)
		{
			small_printf("USB1_OHCI_IRQn interrupt\n\r");
			usb_int_handler();
		}else{
			small_printf("Unknown irq: %d\n\r",irq);
			while(1)
				;
		}
		PLIC->PLIC_MCLAIM_REG = irq;
	}
}

/*
 * PLIC
 */

void irq_enable(int irq)
{
	small_printf("irq_enable %d\n\r", irq);
	PLIC->PLIC_IP_REGn[irq / 32] |= (1 << (irq % 32));
	PLIC->PLIC_PRIO_REGn[irq] = 1;
	PLIC->PLIC_MIE_REGn[irq / 32] |= (1 << (irq % 32));
}

void irq_disable(int irq)
{
	small_printf("irq_disable %d\n\r", irq);
	PLIC->PLIC_MIE_REGn[irq / 32] &= ~(1 << (irq % 32));
}

static void plic_init()
{
	for(int irq = IRQ_NUM_MIN; irq < IRQ_NUM_MAX; irq++)
	{
		PLIC->PLIC_IP_REGn[irq / 32] &= ~(1 << (irq % 32));
		PLIC->PLIC_PRIO_REGn[irq] = 0;
		PLIC->PLIC_MIE_REGn[irq / 32] &= ~(1 << (irq % 32));
	}
}

void irq_init(void)
{
	plic_init();
	csr_set_bits_mie(MIE_MEI_BIT_MASK);	
	csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}
