/*
 * =====================================================================================
 *
 *       Filename:  irq_global.c
 *
 *    Description:  IRQ global control API
 *
 *        Version:  1.0
 *        Created:  12.04.2017 22:25:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <isix/arch/irq_global.h>



#if defined(__ARM_ARCH_7M__)   \
    || defined(__ARM_ARCH_7EM__) \
    || defined(__ARM_ARCH_6M__)
# define THUMB_V7M_V6M
#endif


#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__)
#	define IRQMASK_REG_NAME_R "primask"
#	define IRQMASK_REG_NAME_W "primask"
#	define IRQMASK_I_BIT	1
#else
#	define IRQMASK_REG_NAME_R "cpsr"
#	define IRQMASK_REG_NAME_W "cpsr_c"
#	define IRQMASK_I_BIT	0x00000080
#endif


#ifdef THUMB_V7M_V6M

/** Global IRQ enable */
void isix_irq_enable()
{
	asm volatile(
		"	cpsie i			@ arch_local_irq_enable"
		:
		:
		: "memory", "cc");
}


/** Global IRQ disable */
void isix_irq_disable()
{
	asm volatile(
		"	cpsid i			@ arch_local_irq_disable"
		:
		:
		: "memory", "cc");
}


/** Disable global interrupt and save IRQ status
 * @return Current interrupt mask
 */
unsigned isix_irq_save(void)
{
	unsigned long flags;

	asm volatile(
		"	mrs	%0, " IRQMASK_REG_NAME_R "	@ arch_local_irq_save\n"
		"	cpsid	i"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}


#else /* THUMB_V7M_V6M */
// Normal arm architecture not cortex


/** Global IRQ enable */
void isix_irq_enable()
{
	unsigned long temp;
	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_enable\n"
		"	bic	%0, %0, #128\n"
		"	msr	cpsr_c, %0"
		: "=r" (temp)
		:
		: "memory", "cc");
}


/** Global IRQ disable */
void isix_irq_disable()
{
	unsigned long temp;
	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_disable\n"
		"	orr	%0, %0, #128\n"
		"	msr	cpsr_c, %0"
		: "=r" (temp)
		:
		: "memory", "cc");
}


/** Disable global interrupt and save IRQ status
 * @return Current interrupt mask
 */
unsigned isix_irq_save(void)
{
	unsigned long flags, temp;

	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_save\n"
		"	orr	%1, %0, #128\n"
		"	msr	cpsr_c, %1"
		: "=r" (flags), "=r" (temp)
		:
		: "memory", "cc");
	return flags;
}


#endif /* THUMB_V7M_V6M */


/** Restore global interrupt status
 * @param[in] mask Saved interrupt mask
 */
void isix_irq_restore( unsigned flags )
{
	asm volatile(
		"	msr	" IRQMASK_REG_NAME_W ", %0	@ local_irq_restore"
		:
		: "r" (flags)
		: "memory", "cc");
}

