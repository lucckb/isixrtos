/*
 * fifo.h
 *
 *  Created on: 24-03-2011
 *      Author: lucck
 */

#pragma once
#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

#include <isix/config.h>
#include "semaphore.h"
#include <stdatomic.h>

#if CONFIG_ISIX_FIFO_EVENT_NOTIFY
struct isix_event;
#endif

/* Queue structure */
struct isix_fifo
{
	char *rx_p;     //Pointer to rx
    char *tx_p;     //Pointer to tx
    char *mem_p;    //Pointer to allocated memory
    int size;       //Total fifo size
    int elem_size; //Element count
    struct isix_semaphore rx_sem;  //Semaphore rx
    struct isix_semaphore tx_sem;  //Semaphore for tx
	unsigned flags;	//Extra flags
#if CONFIG_ISIX_FIFO_EVENT_NOTIFY	//! Fifo event notify API
	struct isix_event* evt;
	atomic_uchar bitno;
#endif
};

