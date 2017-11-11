/*
 * =====================================================================================
 *
 *       Filename:  tlsf_isix.c
 *
 *    Description:  TLSF isix wrapper funcs
 *
 *        Version:  1.0
 *        Created:  18.06.2017 19:14:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <isix/memory.h>
#include <isix/prv/scheduler.h>
#include "tlsf.h"

#ifdef CONFIG_ISIX_LOGLEVEL_MEMORY
#	undef CONFIG_ISIX_LOGLEVEL
#	define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_MEMORY
#endif
#include <isix/prv/printk.h>


//! Initialize global heap
void _isixp_alloc_init(void)
{
	extern unsigned char __heap_start;
	extern unsigned char __heap_end;
	size_t ret = init_memory_pool(
			&__heap_end - &__heap_start,
			&__heap_start
	);
	if( ret == (size_t)-1 ) {
		isix_bug("Unable to create TSLF memory pool");
	}
}


//! Get memory statistics
void isix_heap_stats( isix_memory_stat_t* meminfo )
{
	meminfo->used = get_used_size(NULL);
	meminfo->free = get_free_size(NULL);
	meminfo->fragments = 1U;
}


