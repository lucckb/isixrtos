/*
 * mempool.h
 *
 *  Created on: 16-11-2012
 *      Author: lucck
 */
#pragma once
#include <isix/prv/list.h>

/** Private mempool isix structure */
struct isix_mempool
{
    list_entry_t free_elems;
    void *mem;
};

