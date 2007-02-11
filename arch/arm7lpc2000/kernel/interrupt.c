#include <isix/types.h>
#include <asm/interrupt.h>


#define IRQ_MASK 0x00000080
#define FIQ_MASK 0x00000040
#define INT_MASK (IRQ_MASK|FIQ_MASK)


/*-----------------------------------------------------------------------*/
//Get Current program status register
static inline reg_t get_cpsr(void)
{
  reg_t val;
  asm volatile ("mrs %[val], cpsr\n":[val]"=r"(val):);
  return val;
}

/*-----------------------------------------------------------------------*/
//Set Current program status register
static inline void set_cpsr(reg_t val)
{
	asm volatile ("msr  cpsr, %[val]\n" ::[val]"r"(val)  );
}

/*-----------------------------------------------------------------------*/
//Disable interrupt return IRQ state
reg_t irq_disable(void)
{
	reg_t cpsr;
	cpsr = get_cpsr();
	set_cpsr(cpsr | IRQ_MASK);
	return cpsr;
}

/*-----------------------------------------------------------------------*/
//Enable IRQ return previous IRQ state
reg_t irq_enable(void)
{
	reg_t cpsr;
	cpsr = get_cpsr();
	set_cpsr(cpsr & ~IRQ_MASK);
	return cpsr;
}

/*-----------------------------------------------------------------------*/
//Restore previous IRQ state
reg_t irq_restore(reg_t old_cpsr)
{
	reg_t cpsr;
	cpsr = get_cpsr();
	set_cpsr( (cpsr & ~IRQ_MASK) | (old_cpsr & IRQ_MASK) );
	return cpsr;
}

/*-----------------------------------------------------------------------*/
//Disable FIQ interrupt
reg_t fiq_disable(void)
{
	reg_t cpsr;
	cpsr = get_cpsr();
	set_cpsr(cpsr | FIQ_MASK);
	return cpsr;
}

/*-----------------------------------------------------------------------*/
//Enable FIQ interrupt
reg_t fiq_enable(void)
{
	reg_t cpsr;
	cpsr = get_cpsr();
	set_cpsr(cpsr & ~FIQ_MASK);
	return cpsr;
}

/*-----------------------------------------------------------------------*/
//Restore previous FIQ status
reg_t fiq_restore(reg_t old_cpsr)
{
	reg_t cpsr;
	cpsr = get_cpsr();
	set_cpsr( (cpsr & ~FIQ_MASK) | (old_cpsr & FIQ_MASK) );
	return cpsr;
}

/*-----------------------------------------------------------------------*/

