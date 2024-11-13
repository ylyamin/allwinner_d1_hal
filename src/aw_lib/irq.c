/*
 * File: irq.c
 * Author: ylyamin
 */
#include <platform.h>
#include <log.h>

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

static void (*irq_handlers_array[IRQ_NUM_MAX])(void) = {0};

void handle_interrupt(uint64_t mcause);
void handle_exception(uint64_t mcause);

void handle_trap(void)
{
	uint64_t mcause = csr_read_mcause();
	uint64_t mepc = csr_read_mepc();

    if (mcause >> 63) {
		if((mcause & ~(1UL << 63)) < ARRAY_SIZE(interrupt_names))
		{
			LOG_D("Interrupt %d: %s", (mcause & ~(1UL << 63)), interrupt_names[mcause & ~(1UL << 63)]);
			handle_interrupt(mcause);
		}
		else
		{
			LOG_E("Unknown interrupt %d", mcause & ~(1UL << 63));
			while(1)
				;
		}	
    } else {
		if(mcause < ARRAY_SIZE(exception_names))
		{
			LOG_D("Exception %d: %s\r\n", mcause, exception_names[mcause]);
			handle_exception(mcause);
			asm volatile("csrr t0, mepc");
			asm volatile("addi t0, t0, 0x4");
			asm volatile("csrw mepc, t0");
		}
		else
		{
			LOG_E("Unknown exception %d\r\n", mcause);
			while(1)
				;
		}	
    }
}

void handle_exception(uint64_t mcause) {
	LOG_E("Stored pc: %x\r\n", csr_read_mepc());
	LOG_E("Stored mtval: %x\r\n", csr_read_mtval());
	while(1)
		;
}

void handle_interrupt(uint64_t mcause) {
	if((mcause & ~(1UL << 63)) == 11) 	// An external HW interrupt in M-mode
	{
		uint64_t val = csr_read_mip();
		csr_write_mip(val & (uint64_t)(mcause & ~(1UL << 63)));

		uint32_t irq = PLIC->PLIC_MCLAIM_REG;
		if(irq > IRQ_NUM_MIN & irq < IRQ_NUM_MAX)
		{
			LOG_D("Handle irq: %d",irq);
			(irq_handlers_array[irq])();
		}else{
			LOG_E("Unknown irq: %d",irq);
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
	LOG_D("irq_enable %d", irq);
	PLIC->PLIC_IP_REGn[irq / 32] |= (1 << (irq % 32));
	PLIC->PLIC_PRIO_REGn[irq] = 1;
	PLIC->PLIC_MIE_REGn[irq / 32] |= (1 << (irq % 32));
}

void irq_disable(int irq)
{
	LOG_D("irq_disable %d", irq);
	PLIC->PLIC_MIE_REGn[irq / 32] &= ~(1 << (irq % 32));
}

void irq_assign(int irq, void (*func)(void))
{
	if(irq > IRQ_NUM_MIN & irq < IRQ_NUM_MAX)
		irq_handlers_array[irq] = func;
	else
		LOG_E("Unknown irq: %d",irq);
}

void dummy_irq_handler(void)
{
	LOG_W("dummy_irq_handler");
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
	LOG_D("irq_init");
	init_irq_handlers();
	plic_init();
	csr_set_bits_mie(MIE_MEI_BIT_MASK);	
	csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}
