/*
 * mempool.h
 *
 *  Created on: 16-11-2012
 *      Author: lucck
 */
#pragma once
#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

#include <isix/prv/list.h>

/** Private mempool isix structure */
struct isix_mempool
{
    list_entry_t free_elems;
    void *mem;
	size_t mem_size;
};

