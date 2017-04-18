/*
 * =====================================================================================
 *
 *       Filename:  irq_platform.h
 *
 *    Description:  IRQ platform specific details
 *
 *        Version:  1.0
 *        Created:  14.04.2017 21:23:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//! Isix IRQ splited priority
typedef struct isix_irq_prio_s {
	uint8_t prio;
	uint8_t subp;
} isix_irq_prio_t;

//! Isix irq ram
typedef uint8_t isix_irq_raw_prio_t;



//! Standard system CortexM core specific interrupts
enum isix_cortexm_irqnums {
	isix_cortexm_irq_nmi				= -14,
	isix_cortexm_irq_hard_fault			= -13,
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
	isix_cortexm_irq_memory_management  = -12,
	isix_cortexm_irq_bus_fault			= -11,
	isix_cortexm_irq_usage_fault		= -10,
#endif
	isix_cortexm_irq_svc_call			= -5,
	isix_cortexm_irq_pend_svc			= -2,
	isix_cortexm_irq_systick			= -1,
};

/** Mask all interrupts below selected priority level
 * @param[in] priority Input priority
 */
void isix_mask_irq_priority( isix_irq_prio_t priority );


/** Mask priority and save old priority level
 * @param[in] new_prio New masked priority
 * @return old priority
 */
isix_irq_raw_prio_t isix_mask_irq_save_priority( isix_irq_prio_t new_prio );


/** Restore old mask priority previously saved by msr
 * @param[in] saved IRQ priority
 */

static inline void __attribute__((always_inline))
isix_mask_irq_restore_priority( isix_irq_raw_prio_t prio )
{
	asm volatile (
		"msr BASEPRI,%0\n"
		::"r"(prio)
	);
}


/** Umask IRQ priority previously set */
static inline void __attribute__((always_inline))
	isix_umask_irq_priority(void)
{
	isix_mask_irq_restore_priority(0);
}


/** Set irqnumber to the requested priority level
 * @param[in] irqno IRQ input number
 * @param[in] new interrupt priority
 */
void isix_set_irq_priority( int irqno, isix_irq_prio_t priority );


/** Set the raw priority
 * @param[in] irqno IRQ input number
 * @param[in] new interrupt raw priority
 */
void isix_set_raw_priority( int irqno, isix_irq_raw_prio_t prio );


#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

/** Get current active IRQ
 * Interrupt has occurred and is currently being serviced.
 * @return true or false
 */
bool isix_get_active_irq( int irqno );


/** Generate software interrupt
 * @param[in] irqno IRQ input number */
void isix_generate_software_interrupt( int irqno );


#endif

/** Generate event when IRQ pending
 */
void isix_event_irq_pending( void );


#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

//! Enumeration of priority group
enum isix_cortexm_prigroup {
	isix_cortexm_group_pri7_1   =  0x0 << 8,
	isix_cortexm_group_pri7_2   =  0x1 << 8,
	isix_cortexm_group_pri7_3   =  0x2 << 8,
	isix_cortexm_group_pri7_4   =  0x3 << 8,
	isix_cortexm_group_pri7_5   =  0x4 << 8,
	isix_cortexm_group_pri7_6   =  0x5 << 8,
	isix_cortexm_group_pri7     =  0x6 << 8,
	isix_cortexm_group_pri_none =  0x7 << 8,
};


/** Set interrupt priority groupin
 * @param[in] priority group
 */
void isix_set_irq_priority_group( enum isix_cortexm_prigroup prigroup );




#endif

#ifdef __cplusplus
}
#endif
