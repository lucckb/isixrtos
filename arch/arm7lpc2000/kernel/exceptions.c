#include <isix/config.h>
#include <isix/types.h>
#include <isix/printk.h>
#include <asm/ptrace.h>

#define printf printk

/*------------------------------------------------*/

extern void reset_cpu(void);

/*------------------------------------------------*/

void bad_mode (void)
{
	printf ("Halting CPU ... - Please RESET board\n");
	while(1);
}

/*------------------------------------------------*/


void show_regs (struct pt_regs *regs)
{
	unsigned long flags;
	const char *processor_modes[] = {
		"USER_26",  "FIQ_26",   "IRQ_26",   "SVC_26",
		"UK4_26",   "UK5_26",   "UK6_26",   "UK7_26",
		"UK8_26",   "UK9_26",   "UK10_26",  "UK11_26",
		"UK12_26",  "UK13_26",  "UK14_26",  "UK15_26",
		"USER_32",  "FIQ_32",   "IRQ_32",   "SVC_32",
		"UK4_32",   "UK5_32",   "UK6_32",   "ABT_32",
		"UK8_32",   "UK9_32",   "UK10_32",  "UND_32",
		"UK12_32",  "UK13_32",  "UK14_32",  "SYS_32",
	};

	flags = condition_codes (regs);

	printf ("pc : [<%08lx>]    lr : [<%08lx>]\n"
			"sp : %08lx  ip : %08lx  fp : %08lx\n",
			instruction_pointer (regs),
			regs->ARM_lr, regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	printf ("r10: %08lx  r9 : %08lx  r8 : %08lx\n",
			regs->ARM_r10, regs->ARM_r9, regs->ARM_r8);
	printf ("r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",
			regs->ARM_r7, regs->ARM_r6, regs->ARM_r5, regs->ARM_r4);
	printf ("r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",
			regs->ARM_r3, regs->ARM_r2, regs->ARM_r1, regs->ARM_r0);
	printf ("Flags: %c%c%c%c",
			flags & CC_N_BIT ? 'N' : 'n',
			flags & CC_Z_BIT ? 'Z' : 'z',
			flags & CC_C_BIT ? 'C' : 'c', flags & CC_V_BIT ? 'V' : 'v');
	printf ("  IRQs %s  FIQs %s  Mode %s%s\n",
			interrupts_enabled (regs) ? "on" : "off",
			fast_interrupts_enabled (regs) ? "on" : "off",
			processor_modes[processor_mode (regs)],
			thumb_mode (regs) ? " (T)" : "");
}


/*------------------------------------------------*/
//Undefined instruction exception
void undefined_exception(struct pt_regs *pt_regs)
{
    printf ("undefined instruction\n");
	show_regs (pt_regs);
	bad_mode ();
}


/*------------------------------------------------*/
//Data abort exception
void data_abort_exception(struct pt_regs *pt_regs)
{
    printf ("data abort\n");
	show_regs (pt_regs);
	bad_mode ();
}


/*------------------------------------------------*/
//Prefetch abort exception
void prefetch_abort_exception(struct pt_regs *pt_regs)
{
    printf ("prefetch abort\n");
	show_regs (pt_regs);
	bad_mode ();
}

/*------------------------------------------------*/
//Not supported fiq exception
void fiq_exception(struct pt_regs *pt_regs)
{
    printf ("fiq interrupt\n");
	show_regs (pt_regs);
	bad_mode ();

}
