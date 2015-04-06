#include <isix/config.h>
#include <isix/types.h>
#include <isix/memory.h>
#include <isix/semaphore.h>
#include <isix/fifo.h>
#include <isix/prv/semaphore.h>
#include <string.h>
#include <isix/prv/fifo.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
#ifndef ISIX_DEBUG_FIFO
#define ISIX_DEBUG_FIFO ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_FIFO == ISIX_DBG_ON
#include <isix/printk.h>
#else
#undef printk
#define printk(...) do {} while(0)
#endif

/*-------------------------------------------------------*/
//! Private function for lock
static inline __attribute__((always_inline)) void _lock( const osfifo_t fifo )
{
	if( fifo->flags & isix_fifo_f_noirq ) {
		_isixp_lock_scheduler();
	} else {
		_isixp_enter_critical();
	}
}
/*-------------------------------------------------------*/
//! Private function for unlock
static inline __attribute__((always_inline)) void _unlock( const osfifo_t fifo )
{
	if( fifo->flags & isix_fifo_f_noirq ) {
		_isixp_unlock_scheduler();
	} else {
		_isixp_exit_critical();
	}
}
/*-------------------------------------------------------*/
/* Create queue for n elements
 * if succes return queue pointer else return null
 */

osfifo_t isix_fifo_create_ex( int n_elem, int elem_size, unsigned flags )
{
   //Create fifo struct
   osfifo_t fifo = (osfifo_t)isix_alloc(sizeof(struct isix_fifo));
   if(!fifo)
   {
       printk("FifoCreate: Error alloc fifo struct");
       return NULL;
   }
   //Set type
   fifo->type = osobject_type_fifo;
   //Set flags
   fifo->flags = flags;
   //Calculate size
   fifo->size = n_elem * elem_size;
   //Create used memory struct
   fifo->mem_p = (char*)isix_alloc(fifo->size);
   if(!fifo->mem_p)
   {
     printk("FifoCreate: Error alloc data");
     isix_free(fifo);
     return NULL;
   }
   //Fill element of structure
   fifo->elem_size = elem_size;
   fifo->rx_p = fifo->tx_p = fifo->mem_p;
   //Create RX sem as 0 element in fifo (task sleep)
   isix_sem_create(&fifo->rx_sem,0);
   //Create tx sem as numer of element in fifo
   isix_sem_create(&fifo->tx_sem,n_elem);
   printk("FifoCreate New fifo handler %p",fifo);
   return fifo;
}

/*----------------------------------------------------------------*/
//Fifo send to other task
int isix_fifo_write(osfifo_t fifo,const void *item, ostick_t timeout)
{
    if(!fifo) return ISIX_EINVARG;
    if(isix_sem_wait(&fifo->tx_sem,timeout)<0)
    {
        printk("FifoWrite: Timeout on TX queue");
        return ISIX_ETIMEOUT;
    }
    _lock(fifo);
    memcpy(fifo->tx_p,item,fifo->elem_size);
    printk("FifoWrite: Data write at TXp %p",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    _unlock(fifo);
    printk("FifoWrite: New TXp %p",fifo->tx_p);
    //Signaling RX thread with new data
    return isix_sem_signal(&fifo->rx_sem);
}
/*----------------------------------------------------------------*/
//Fifo send to other task
int isix_fifo_write_isr(osfifo_t fifo,const void *item)
{
    if(!fifo) return ISIX_EINVARG;
	if(fifo->flags & isix_fifo_f_noirq) return ISIX_EINVARG;
    if(isix_sem_get_isr(&fifo->tx_sem)<0)
    {
        printk("FifoWriteISR: No space in TX queue");
        return ISIX_EFIFOFULL;
    }
    _lock(fifo);
    memcpy(fifo->tx_p,item,fifo->elem_size);
    printk("FifoWriteISR: Data write at TXp %p",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    _unlock(fifo);
    printk("FifoWriteISR: New TXp %p",fifo->tx_p);
    //Signaling RX thread with new data
    return isix_sem_signal_isr(&fifo->rx_sem);
}
/*----------------------------------------------------------------*/
//Fifo receive from other task
int isix_fifo_read(osfifo_t fifo,void *item, ostick_t timeout)
{
    if(!fifo) return ISIX_EINVARG;
    if(isix_sem_wait(&fifo->rx_sem,timeout)<0)
    {
       printk("FifoRead: Timeout on RX queue");
       return ISIX_ETIMEOUT;
    }
    _lock(fifo);
    memcpy(item,fifo->rx_p,fifo->elem_size);
    printk("FifoRead: Data write at RXp %p",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    _unlock(fifo);
    printk("FifoRead: New Rxp %p",fifo->rx_p);
    //Signaling TX for space avail
    return isix_sem_signal(&fifo->tx_sem);
}

/*----------------------------------------------------------------*/
//Fifo receive from other task
int isix_fifo_read_isr(osfifo_t fifo,void *item)
{
    if(!fifo) return ISIX_EINVARG;
	if(fifo->flags & isix_fifo_f_noirq) return ISIX_EINVARG;
    if(isix_sem_get_isr(&fifo->rx_sem)<0)
    {
       printk("FifoReadISR: No space in RX queue");
       return ISIX_EFIFOFULL;
    }
    _lock(fifo);
    memcpy(item,fifo->rx_p,fifo->elem_size);
    printk("FifoReadISR: Data write at RXp %p",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    _unlock(fifo);
    printk("FifoReadISR: New Rxp %p",fifo->rx_p);
    //Signaling TX for space avail
    return isix_sem_signal_isr(&fifo->tx_sem);
}

/*----------------------------------------------------------------*/
/* Delete created queue */
int isix_fifo_destroy(osfifo_t fifo)
{
    _lock(fifo);
    //Check for TXSEM ban be destroyed
    if(_isixp_sem_can_destroy(&fifo->tx_sem)==false)
    {
        printk("FifoDestroy: Error TXSem busy");
        _unlock(fifo);
        return ISIX_EBUSY;
    }
    //Check for RXSEM can be destroyed
    if(_isixp_sem_can_destroy(&fifo->rx_sem)==false)
    {
        printk("FifoDestroy: Error RXSem busy");
        _unlock(fifo);
        return ISIX_EBUSY;
    }
    //Destroy RXSEM and TXSEM
    isix_sem_destroy(&fifo->rx_sem);
    isix_sem_destroy(&fifo->tx_sem);
    //Free queue used memory
    isix_free(fifo->mem_p);
    isix_free(fifo);
    _unlock(fifo);
    return ISIX_EOK;
}

/*----------------------------------------------------------------*/
//How many element is in fifo
int isix_fifo_count(osfifo_t fifo)
{
    if(!fifo) return ISIX_EINVARG;
    return isix_sem_getval(&fifo->rx_sem);
}

/*----------------------------------------------------------------*/

