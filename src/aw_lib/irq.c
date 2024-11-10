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

static int (*irq_handlers_array[IRQ_NUM_MAX])(void) = {0};

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
	small_printf("Stored pc: %x\r\n", csr_read_mepc());
	while(1)
		;
}

void handle_interrupt(uint64_t mcause) {
	// An external HW interrupt in M-mode
	if((mcause & ~(1UL << 63)) == 11)
	{
		uint64_t val = csr_read_mip();
		csr_write_mip(val & (uint64_t)(mcause & ~(1UL << 63)));

		uint32_t irq = PLIC->PLIC_MCLAIM_REG;
		if(irq > IRQ_NUM_MIN & irq < IRQ_NUM_MAX)
		{
			small_printf("Handle irq: %d\n\r",irq);
			(irq_handlers_array[irq])();
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

void irq_assign(int irq, void (*func)(void *))
{
	if(irq > IRQ_NUM_MIN & irq < IRQ_NUM_MAX)
		irq_handlers_array[irq] = func;
	else
		small_printf("Unknown irq: %d\n\r",irq);
}

void dummy_irq_handler(void)
{
	small_printf("dummy_irq_handler\n\r");
}

void init_irq_handlers(void)
{
 	for(int i = 0; i < ARRAY_SIZE(irq_handlers_array); i++)
	{
		irq_handlers_array[i] = dummy_irq_handler;
	}	
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
	init_irq_handlers();
	plic_init();
	csr_set_bits_mie(MIE_MEI_BIT_MASK);	
	csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}
