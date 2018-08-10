/*
 * =====================================================================================
 *
 *       Filename:  irq.h
 *
 *    Description:  IRQ core controller device API
 *
 *        Version:  1.0
 *        Created:  12.04.2017 20:32:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <isix/arch/irq_platform.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/** Request selected interrupt handler
 * @param[in] irqno IRQ specific to the hardware
 */
void isix_request_irq( int irqno );


/** Unblock and disable selected irq number */
void isix_free_irq( int irqno );


/** Check if IRQ is active
 * @param[in] irqno IRQ input number
 * @return boolean yes or not
 */
bool isix_get_irq_enabled( int irqno );


/** Check if IRQ is pending
 * @param[in] irqno IRQ input numer
 * @return boolean yes or not
 */
bool isix_get_irq_pending( int irqno );


/** SET pending IRQ state
 * @param[in] irqno IRQ input number
 */
void isix_set_irq_pending( int irqno );


/** CLEAR pending IRQ state
 * @param[in] irqno IRQ input number
 */
void isix_clear_irq_pending( int irqno );




#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
namespace isix {
	/** Request selected interrupt handler
	 * @param[in] irqno IRQ specific to the hardware
	 */
	inline __attribute__((always_inline))
	void request_irq(int irqno) {
		::isix_request_irq(irqno);
	}

	/** Unblock and disable selected irq number */
	inline __attribute__((always_inline))
	void free_irq(int irqno) {
		::isix_free_irq(irqno);
	}

	/** Check if IRQ is active
	 * @param[in] irqno IRQ input number
	 * @return boolean yes or not
	 */
	inline __attribute__((always_inline))
	bool get_irq_enabled(int irqno) {
		return ::isix_get_irq_enabled(irqno);
	}

	/** Check if IRQ is pending
	 * @param[in] irqno IRQ input numer
	 * @return boolean yes or not
	 */
	inline __attribute__((always_inline))
	bool get_irq_pending(int irqno) {
		return ::isix_get_irq_pending(irqno);
	}

	/** SET pending IRQ state
	 * @param[in] irqno IRQ input number
	 */
	inline __attribute__((always_inline))
	void set_irq_pending( int irqno ) {
		::isix_set_irq_pending(irqno);
	}

	/** CLEAR pending IRQ state
	 * @param[in] irqno IRQ input number
	 */
	inline __attribute__((always_inline))
	void clear_irq_pending(int irqno) {
		::isix_clear_irq_pending(irqno);
	}
}
#endif


