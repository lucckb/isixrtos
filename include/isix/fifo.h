/* Queue operations */
#ifndef __ISIX_FIFO_H
#define __ISIX_FIFO_H

/*--------------------------------------------------*/
/* Queue structure */
typedef struct fifo_struct
{
    char *rx_p;     //Pointer to rx
    char *tx_p;     //Pointer to tx
    char *mem_p;    //Pointer to allocated memory
    int size;       //Total fifo size
    int elem_size; //Element count
    sem_t *rx_sem;  //Semaphore rx
    sem_t *tx_sem;  //Semaphore for tx
} fifo_t;


/*--------------------------------------------------*/
/* Create queue for n elements
 * if succes return queue pointer else return null   */
fifo_t* fifo_create_isr(int n_elem, int elem_size,s8 interrupt);

#define fifo_create(n_elem,elem_size) fifo_create_isr(n_elem,elem_size,-1)

/* Send element to queue
 * queue - Pointer to queue
 * item -  Element to push in queue
 * timeout - Timeout for selected queue
 * return 0 if succes else return error     */
int fifo_write(fifo_t *fifo,const void *item,unsigned long timeout);

int fifo_write_isr(fifo_t *queue,const void *item);

/*--------------------------------------------------*/
/* Delete created queue */
int fifo_destroy(fifo_t *fifo);

/*--------------------------------------------------*/
/* How many elements is in fifo */
int fifo_count(fifo_t *fifo);

/*--------------------------------------------------*/
/* Get element from queue *
 * queue - Pointer to queue
 * item - Pointer to item
 * timeout - Max waiting timeouts */
int fifo_read(fifo_t *fifo,void *item,unsigned long timeout);

int fifo_read_isr(fifo_t *queue,void *item);

/*--------------------------------------------------*/

#endif
