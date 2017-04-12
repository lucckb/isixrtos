/*
 * =====================================================================================
 *
 *       Filename:  irq_global.h
 *
 *    Description:  IRQ global control API
 *
 *        Version:  1.0
 *        Created:  12.04.2017 22:22:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

/** Global IRQ enable */
void isix_irq_enable();


/** Global IRQ disable */
void isix_irq_disable();


/** Disable global interrupt and save IRQ status
 * @return Current interrupt mask
 */
unsigned isix_irq_save(void);


/** Restore global interrupt status
 * @param[in] mask Saved interrupt mask
 */
void isix_irq_restore( unsigned mask );

#ifdef __cplusplus
}
#endif

