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

#ifndef __cplusplus
extern "C" {
#endif

/** Request selected interrupt handler
 * @param[in] irqno IRQ specific to the hardware
 * @param[in] attr  Attribute specific to the hardware
 */
void isix_request_irq( int irqno, unsigned attr );


/** Unblock and disable selected irq number */
void isix_free_irq( int irqno );



#ifndef __cplusplus
}
#endif
