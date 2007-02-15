#include <isix/types.h>
#include <asm/interrupt.h>
#include <asm/lpc214x.h>
#include <asm/lpc214x_vic.h>

#define DEBUG

#ifdef DEBUG
#include <isix/printk.h>
#else
#define printk(...)
#endif

#define IRQ_MASK 0x00000080
#define FIQ_MASK 0x00000040
#define INT_MASK (IRQ_MASK|FIQ_MASK)


/*-----------------------------------------------------------------------*/
//Variable holds used vectorized slots
static volatile u16 used_irq_slots = 0;

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
//Register irq interrupt
int interrupt_register(u8 int_num,s16 prio,interrupt_proc_ptr_t interrupt_proc )
{
    //If priority is invalid then exit
    if( (prio<INTERRUPT_PRIO_FIRST && prio>INTERRUPT_PRIO_LAST)
         && prio!=INTERRUPT_PRIO_DEFAULT )
    {
        printk("InterruptRegister: Priority %d is invalid\n",prio);
        return -1;
    }
    //Slot is used??
    if(prio!=INTERRUPT_PRIO_DEFAULT)
    {
        if( used_irq_slots & (1<<prio) )
        {
            printk("InterruptRegister: Slot %d is used\n",prio);
            return -2;
        }
    }
    //Disable interrupt in CPU
    reg_t irq_s;
    irq_s = irq_disable();
    //Setup vector addres according to priority
    volatile reg_t *vectaddr = prio!=INTERRUPT_PRIO_DEFAULT?(&VICVectAddr0):(&VICDefVectAddr);
    //Cntl Register
    volatile reg_t *cntl = &VICVectCntl0;
    printk("InterruptRegister: VectAddr %08x CntlAddr %08x\n",vectaddr,cntl);
    //Setup interrupt vector
    vectaddr[prio] = (u32)interrupt_proc;
    //If reg is default
    if(prio!=INTERRUPT_PRIO_DEFAULT)
    {
        //Enable vect slot
        cntl[prio] = (int_num & VIC_CNTLREG_MASK) | VIC_CNTLREG_SLOTEN;
        used_irq_slots |= 1<<prio;
    }
    //Enable interrupt
    VICIntEnable = 1<<int_num;
    //Enable interrupt in cpu
    irq_restore(irq_s);
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Unregister specified interrupt */
int interrupt_unregister(u8 int_num)
{
    volatile reg_t *cntl = &VICVectCntl0;
   //Try find interrupt in selected slots
   reg_t irq_s = irq_disable();
    for(int i=0;i<16;i++)
   {
     if( (cntl[i]&VIC_CNTLREG_MASK)==int_num && (cntl[i]&VIC_CNTLREG_SLOTEN) )
     {
        //Znaleziono slot posiadajacy przerwanie o tym numerze
        used_irq_slots &= (~0x1)<<i;
        cntl[i] = 0;
     }
   }
   VICIntEnClr = (1<<int_num);
   irq_restore(irq_s);
   return 0;
}
/*-----------------------------------------------------------------------*/
//Register fiq interrupt (arm specific issue)
int interrupt_register_fiq(u8 int_num)
{
   reg_t fiq_s = fiq_disable();
   //Enable fiq interrupt
   VICIntSelect |= 1<<int_num;
   VICIntEnable = 1<<int_num;
   fiq_restore(fiq_s);
   return 0;
}

/*-----------------------------------------------------------------------*/
//Mask or unmask selected interrupt
int interrupt_mask(u8 int_num,bool set_clr)
{
    reg_t fiq_s,irq_s;
    fiq_s = fiq_disable();
    irq_s = irq_disable();
    if(set_clr==true)  VICIntEnable = 1<<int_num;
    else VICIntEnClr = 1<<int_num;
    fiq_restore(fiq_s);
    irq_restore(irq_s);
    return 0;
}
/*-----------------------------------------------------------------------*/

