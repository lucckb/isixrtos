/*
 * fifo.h
 *
 *  Created on: 24-03-2011
 *      Author: lucck
 */

#pragma once

#include "semaphore.h"


/* Queue structure */
struct fifo_struct
{
	char *rx_p;     //Pointer to rx
    char *tx_p;     //Pointer to tx
    char *mem_p;    //Pointer to allocated memory
    int size;       //Total fifo size
    int elem_size; //Element count
    struct sem_struct rx_sem;  //Semaphore rx
    struct sem_struct tx_sem;  //Semaphore for tx
	unsigned flags;	//Extra flags
};

