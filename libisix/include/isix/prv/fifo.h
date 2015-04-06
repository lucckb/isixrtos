/*
 * fifo.h
 *
 *  Created on: 24-03-2011
 *      Author: lucck
 */

#pragma once

#include "semaphore.h"


/* Queue structure */
struct isix_fifo
{
	osobject_type_t type;	//Type of object
	char *rx_p;     //Pointer to rx
    char *tx_p;     //Pointer to tx
    char *mem_p;    //Pointer to allocated memory
    int size;       //Total fifo size
    int elem_size; //Element count
    struct isix_semaphore rx_sem;  //Semaphore rx
    struct isix_semaphore tx_sem;  //Semaphore for tx
	unsigned flags;	//Extra flags
};

