/*
 * fifo.h
 *
 *  Created on: 24-03-2011
 *      Author: lucck
 */

#pragma once

#include <isix/config.h>
#include "semaphore.h"
#include <stdatomic.h>

#ifdef CONFIG_ISIX_FIFO_EVENT_NOTIFY
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
#ifdef CONFIG_ISIX_FIFO_EVENT_NOTIFY	//! Fifo event notify API
	struct isix_event* evt;
	atomic_uchar bitno;
#endif
};

