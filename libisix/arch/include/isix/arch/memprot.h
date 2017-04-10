/*
 * =====================================================================================
 *
 *       Filename:  _isix_port_memprot.h
 *
 *    Description:  Port mem protection header
 *
 *        Version:  1.0
 *        Created:  06.05.2015 19:46:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <isix/config.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/** Function initialize default memory protection layout just before run
 */
#if CONFIG_ISIX_MEMORY_PROTECTION_MODEL > 0
//! Define memory protection electric fence length
#define ISIX_MEMORY_PROTECTION_EFENCE_SIZE 32

/** Function initialize default memory protection layout just before run */
void _isix_port_memory_protection_set_default_map(void);

/**  Set electric fence on the selected address
 *   this function is used by rtos to protect general heap
 *   memory region  it must be 32 byte aligned
 *   @param[in] endstack Set address
 */
void _isix_port_memory_protection_set_efence( uintptr_t endstack );

//! Clear the memory protection efence
void _isix_port_memory_protection_reset_efence(void);

//! Return the MPU properly alligned region
static inline
uintptr_t _isix_port_memory_efence_aligna( uintptr_t addr )
{
	// Fence region must be inside of alloated space
	if( addr & (ISIX_MEMORY_PROTECTION_EFENCE_SIZE-1) ) {
#ifndef CONFIG_ISIX_STACK_ASCENDING
		addr += ISIX_MEMORY_PROTECTION_EFENCE_SIZE;
#else
		addr -= ISIX_MEMORY_PROTECTION_EFENCE_SIZE;
#endif
	}
	addr -= addr&(ISIX_MEMORY_PROTECTION_EFENCE_SIZE-1);
	return addr;
}

#else /* CONFIG_ISIX_MEMORY_PROTECTION_MODEL */

//! Define memory protection electric fence length
#define ISIX_MEMORY_PROTECTION_EFENCE_SIZE 0
#define _isix_port_memory_protection_set_default_map() do {} while(0)
#define _isix_port_memory_set_efence(a) do {} while(0)
#define _isix_port_memory_protection_reset_efence() do {} while(0)
static inline __attribute__((always_inline))
uintptr_t  _isix_port_memory_efence_aligna( uintptr_t addr )
{
	return addr;
}
#endif /* CONFIG_ISIX_MEMORY_PROTECTION_MODEL  */


