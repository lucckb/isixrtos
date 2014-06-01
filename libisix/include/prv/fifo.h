/*
 * fifo.h
 *
 *  Created on: 24-03-2011
 *      Author: lucck
 */

/*-------------------------------------------------------*/
#ifndef ISIX_PRV_FIFO_H_
#define ISIX_PRV_FIFO_H_
/*-------------------------------------------------------*/

/* Queue structure */
struct fifo_struct
{
	char *rx_p;     //Pointer to rx
    char *tx_p;     //Pointer to tx
    char *mem_p;    //Pointer to allocated memory
    int size;       //Total fifo size
    int elem_size; //Element count
    sem_t rx_sem;  //Semaphore rx
    sem_t tx_sem;  //Semaphore for tx
	unsigned flags;	//Extra flags
};


/*-------------------------------------------------------*/
#endif /* FIFO_H_ */
