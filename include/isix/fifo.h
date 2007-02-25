/* Queue operations */
#ifndef __ISIX_FIFO_H
#define __ISIX_FIFO_H

/* Queue structure */
typedef struct fifo_struct
{
    int tmp;
} fifo_t;



/* Create queue for n elements
 * if succes return queue pointer else return null   */
fifo_t * fifo_create(int n_elem, int elem_size);

/* Send element to queue
 * queue - Pointer to queue
 * item -  Element to push in queue
 * timeout - Timeout for selected queue
 * return 0 if succes else return error     */
int fifo_send(fifo_t *fifo,const void *item,unsigned long timeout);

int fifo_send_isr(fifo_t *queue,const void *item);

/* Get count of messages in queue */
int fifo_get_message_count(fifo_t *fifo);

/* Delete created queue */
void fifo_delete(fifo_t *fifo);

/* Get element from queue *
 * queue - Pointer to queue
 * item - Pointer to item
 * timeout - Max waiting timeouts */
int fifo_receive(fifo_t *fifo,void *item,unsigned long timeout);

int fifo_receive_isr(fifo_t *queue,void *item);

#endif
