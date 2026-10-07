/*
 * =====================================================================================
 *
 *       Filename:  irq_platform.c
 *
 *    Description:  IRQ platform implementation API
 *
 *        Version:  1.0
 *        Created:  15.04.2017 18:21:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <isix/arch/irq.h>
#include <isix/arch/io.h>
#include <isix/arch/scheduler.h>
#include <isix/config.h>
#include <isix/cortexm/nvic_regs.h>
#include <isix/cortexm/scb_regs.h>
#include <isix/cortexm/scs_regs.h>


#ifndef CONFIG_ISIX_NVIC_PRIO_BITS
#define CONFIG_ISIX_NVIC_PRIO_BITS 4
#endif

_Static_assert(CONFIG_ISIX_NVIC_PRIO_BITS >= 2 && CONFIG_ISIX_NVIC_PRIO_BITS <= 8,
	"Unsupported number of implemented NVIC priority bits");

_Static_assert(ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY > 0 && ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY <= 0xFF,
	"Kernel interrupt mask priority out of range");
_Static_assert(((unsigned int)ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY & ((1U << (8U - CONFIG_ISIX_NVIC_PRIO_BITS)) - 1U)) == 0,
	"Kernel interrupt mask priority uses not implemented NVIC priority bits");

/* Returns the priority aligned to the upper bits, as stored in IPR/SHPR/BASEPRI */
static uint32_t nvic_encode_prio( isix_irq_prio_t prio )
{
	uint32_t pgrp = ((SCB_AIRCR) & (uint32_t)0x700) >> 8;

	uint32_t prio_bits =
		((7UL - pgrp) > (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS))
		? (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS)
		: (uint32_t)(7UL - pgrp);

	uint32_t sub_bits  =
		((pgrp + (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS)) < (uint32_t)7UL)
		? (uint32_t)0UL
		: (uint32_t)((pgrp - 7UL) + (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS));

	const uint32_t encoded =
			((prio.prio & (uint32_t)((1UL << (prio_bits)) - 1UL)) << sub_bits) |
			((prio.subp     & (uint32_t)((1UL << (sub_bits    )) - 1UL)));
	return (encoded << (8U - (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS))) & 0xFFU;
}


/* 
static isix_irq_prio_t nvic_decode_prio( uint32_t raw_prio )
{
	uint32_t pgrp = ((SCB_AIRCR) & (uint32_t)0x700) >> 8;
	uint32_t prio_bits  = ((7UL - pgrp) > (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS))
		? (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS)
		: (uint32_t)(7UL - pgrp);
	uint32_t sub_bits = ((pgrp + (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS)) < (uint32_t)7UL)
		? (uint32_t)0UL
		: (uint32_t)((pgrp - 7UL) + (uint32_t)(CONFIG_ISIX_NVIC_PRIO_BITS));

	isix_irq_prio_t ret;
	ret.prio = (raw_prio >> sub_bits) & (uint32_t)((1UL << (prio_bits)) - 1UL);
	ret.subp = (raw_prio ) & (uint32_t)((1UL << (sub_bits    )) - 1UL);
	return ret;
}
*/

/** Convert isix priority to raw prioity
 * @param[in] prio Decoded priority
 * @return Raw priority
 */
isix_irq_raw_prio_t isix_irq_priority_to_raw_priority( isix_irq_prio_t prio )
{
	return nvic_encode_prio(prio);
}


/** Request selected interrupt handler
 * @param[in] irqno IRQ specific to the hardware
 */
void isix_request_irq( int irqno )
{
	NVIC_ISER(irqno / 32) = 1U << (irqno % 32);
}


/** Unblock and disable selected irq number */
void isix_free_irq( int irqno )
{
	NVIC_ICER(irqno / 32) =  1U << (irqno % 32);
}



/** Check if IRQ is active
 * @param[in] irqno IRQ input number
 * @return boolean yes or not
 */
bool isix_get_irq_enabled( int irqno )
{
	return NVIC_ISER(irqno / 32) & (1U << (irqno % 32U)) ? true : false;
}


/** Check if IRQ is pending
 * @param[in] irqno IRQ input numer
 * @return boolean yes or not
 */
bool isix_get_irq_pending( int irqno )
{
	return NVIC_ISPR(irqno / 32) & (1U << (irqno % 32U)) ? true : false;
}


/** SET pending IRQ state
 * @param[in] irqno IRQ input number
 */
void isix_set_irq_pending( int irqno )
{
	NVIC_ISPR(irqno / 32) =  1U << (irqno % 32);
}


/** CLEAR pending IRQ state
 * @param[in] irqno IRQ input number
 */
void isix_clear_irq_pending( int irqno )
{
	NVIC_ICPR(irqno / 32) =  1U << (irqno % 32);
}

/** Mask all interrupts below selected priority level
 * @param[in] priority Input priority
 */
void isix_mask_irq_priority( isix_irq_prio_t priority )
{
	isix_mask_irq_restore_priority( nvic_encode_prio( priority ) );
}


/** Mask priority and save old priority level
 * @param[in] new_prio New masked priority
 * @return old priority
 */
isix_irq_raw_prio_t isix_mask_irq_save_priority( isix_irq_prio_t new_prio )
{
	uint32_t prio = nvic_encode_prio( new_prio );
	uint32_t ret;
	asm volatile (
			"mrs %[result], BASEPRI\n"
			"msr BASEPRI,%[inprio]\n"
			"isb\n"
			: [result] "=&r" ( ret )
			: [inprio] "r"  ( prio )
			: "memory"
	);
	return ret;
}


/** Check if the interrupt with the raw priority is masked by the kernel critical section
 * @param[in] raw Raw interrupt priority
 * @return true when the interrupt cannot preempt the kernel
 */
bool isix_irq_raw_priority_is_kernel_masked( isix_irq_raw_prio_t raw )
{
	const uint32_t pgrp = ((SCB_AIRCR) & (uint32_t)0x700) >> 8;
	const uint32_t group_mask = ~((2UL << pgrp) - 1UL) & 0xFFU;
	return ((uint32_t)raw & group_mask) >= ((uint32_t)ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY & group_mask);
}


/** Set the raw priority
 * @param[in] irqno IRQ input number
 * @param[in] new interrupt raw priority
 */
void isix_set_raw_irq_priority( int irqno, isix_irq_raw_prio_t prio )
{
	if (irqno < 0 ) {
		/* Cortex-M  system interrupts */
		SCS_SHPR((irqno & 0xF) - 4) = prio;
	} else {
		/* Device specific interrupts */
		//WARNING: 8bit IO
		NVIC_IPR(irqno) = prio;
	}
}

/** Get the raw priority
 * @param[in] irqno IRQ input number
 * @return Current interrupt raw priority
 */
isix_irq_raw_prio_t isix_get_raw_irq_priority( int irqno )
{
	if (irqno < 0 ) {
		/* Cortex-M  system interrupts */
		return SCS_SHPR((irqno & 0xF) - 4);
	} else {
		/* Device specific interrupts */
		//WARNING: 8bit IO
		return NVIC_IPR(irqno);
	}
}

/** Set irqnumber to the requested priority level
 * @param[in] irqno IRQ input number
 * @param[in] new interrupt priority
 */
void isix_set_irq_priority( int irqno, isix_irq_prio_t priority )
{
	uint8_t rawprio = nvic_encode_prio( priority );
	isix_set_raw_irq_priority( irqno, rawprio );
}


#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

/** Get current active IRQ
 * Interrupt has occurred and is currently being serviced.
 * @return true or false
 */
bool isix_get_active_irq( int irqno )
{
	return NVIC_IABR(irqno / 32) & (1U << (irqno % 32U)) ? true : false;
}

/** Generate software interrupt
 * @param[in] irqno IRQ input number */
void isix_generate_software_interrupt( int irqno )
{
	if (irqno <= 239) {
		NVIC_STIR |= irqno;
	}
}


/** Generate event when IRQ pending
 */
void isix_event_irq_pending(bool en)
{
	if(en)
		SCB_SCR |= SCB_SCR_SEVEONPEND;
	else
		SCB_SCR &= ~SCB_SCR_SEVEONPEND;
}


/** Set interrupt priority groupin
 * @param[in] priority group
 */
void isix_set_irq_priority_group( enum isix_cortexm_prigroup prigroup )
{
	SCB_AIRCR = SCB_AIRCR_VECTKEY | prigroup;
}


#endif


/* This is a part of porting layer */
void _isix_port_system_reset( void )
{
	SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
	for(;;);
}


/** Set the vector base address
 * @param[in] addr memory base starting address
 */
void isix_set_irq_vectors_base( const void* vecptr )
{
	SCB_VTOR = (uintptr_t)vecptr & ~0x7fU;
}


