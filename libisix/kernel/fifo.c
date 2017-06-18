#include <isix/config.h>
#include <isix/types.h>
#include <isix/memory.h>
#include <isix/semaphore.h>
#include <isix/fifo.h>
#include <isix/prv/semaphore.h>
#include <string.h>
#include <isix/prv/fifo_lock.h>
#include <isix/prv/scheduler.h>
#ifndef ISIX_DEBUG_FIFO
#define ISIX_DEBUG_FIFO ISIX_DBG_OFF
#endif


#ifdef CONFIG_ISIX_LOGLEVEL_FIFO
#undef CONFIG_ISIX_LOGLEVEL 
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_FIFO
#endif
#include <isix/prv/printk.h>



/* Create queue for n elements
 * if succes return queue pointer else return null
 */
osfifo_t isix_fifo_create_ex( int n_elem, int elem_size, unsigned flags )
{
   //Create fifo struct
   osfifo_t fifo = (osfifo_t)isix_alloc(sizeof(struct isix_fifo));
   if(!fifo)
   {
       pr_err("FifoCreate: Error alloc fifo struct");
       return NULL;
   }
   memset( fifo, 0, sizeof(struct isix_fifo) );
   //Set flags
   fifo->flags = flags;
   //Calculate size
   fifo->size = n_elem * elem_size;
   //Create used memory struct
   fifo->mem_p = (char*)isix_alloc(fifo->size);
   if(!fifo->mem_p)
   {
     pr_err("FifoCreate: Error alloc data");
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
   pr_info("FifoCreate New fifo handler %p",fifo);
   return fifo;
}


//Fifo send to other task
int isix_fifo_write(osfifo_t fifo,const void *item, ostick_t timeout)
{
    if(!fifo) return ISIX_EINVARG;
    if(isix_sem_wait(&fifo->tx_sem,timeout)<0)
    {
        pr_info("FifoWrite: Timeout on TX queue");
        return ISIX_ETIMEOUT;
    }
    _fifo_lock(fifo);
    memcpy(fifo->tx_p,item,fifo->elem_size);
    pr_debug("FifoWrite: Data write at TXp %p",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    _fifo_unlock(fifo);
    pr_debug("FifoWrite: New TXp %p",fifo->tx_p);
    //Signaling RX thread with new data
	_isixp_fifo_rxavail_event_raise( fifo, false );
    return isix_sem_signal(&fifo->rx_sem);
}

//Fifo send to other task
int isix_fifo_write_isr(osfifo_t fifo,const void *item)
{
    if(!fifo) return ISIX_EINVARG;
	if(fifo->flags & isix_fifo_f_noirq) return ISIX_EINVARG;
    if(isix_sem_trywait(&fifo->tx_sem)<0)
    {
        pr_err("FifoWriteISR: No space in TX queue");
        return ISIX_EFIFOFULL;
    }
    _fifo_lock(fifo);
    memcpy(fifo->tx_p,item,fifo->elem_size);
    pr_debug("FifoWriteISR: Data write at TXp %p",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    _fifo_unlock(fifo);
    pr_debug("FifoWriteISR: New TXp %p",fifo->tx_p);
    //Signaling RX thread with new data
	_isixp_fifo_rxavail_event_raise( fifo, true );
    return isix_sem_signal_isr(&fifo->rx_sem);
}

//Fifo receive from other task
int isix_fifo_read(osfifo_t fifo,void *item, ostick_t timeout)
{
    if(!fifo) return ISIX_EINVARG;
    if(isix_sem_wait(&fifo->rx_sem,timeout)<0)
    {
       pr_err("FifoRead: Timeout on RX queue");
       return ISIX_ETIMEOUT;
    }
    _fifo_lock(fifo);
    memcpy(item,fifo->rx_p,fifo->elem_size);
    pr_debug("FifoRead: Data write at RXp %p",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    _fifo_unlock(fifo);
    pr_debug("FifoRead: New Rxp %p",fifo->rx_p);
    //Signaling TX for space avail
    return isix_sem_signal(&fifo->tx_sem);
}


//Fifo receive from other task
int isix_fifo_read_isr(osfifo_t fifo,void *item)
{
    if(!fifo) return ISIX_EINVARG;
	if(fifo->flags & isix_fifo_f_noirq) return ISIX_EINVARG;
    if(isix_sem_trywait(&fifo->rx_sem)<0)
    {
       pr_err("FifoReadISR: No data waiting");
       return ISIX_EFIFOEMPTY;
    }
    _fifo_lock(fifo);
    memcpy(item,fifo->rx_p,fifo->elem_size);
    pr_debug("FifoReadISR: Data write at RXp %p",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    _fifo_unlock(fifo);
    pr_debug("FifoReadISR: New Rxp %p",fifo->rx_p);
    //Signaling TX for space avail
    return isix_sem_signal_isr(&fifo->tx_sem);
}


/* Delete created queue */
int isix_fifo_destroy(osfifo_t fifo)
{
    _fifo_lock(fifo);
    //Destroy RXSEM and TXSEM
    isix_sem_destroy(&fifo->rx_sem);
    isix_sem_destroy(&fifo->tx_sem);
    _fifo_unlock(fifo);
    //Free queue used memory
    isix_free(fifo->mem_p);
    isix_free(fifo);
    return ISIX_EOK;
}


//How many element is in fifo
int isix_fifo_count(osfifo_t fifo)
{
    if(!fifo) return ISIX_EINVARG;
    return isix_sem_getval(&fifo->rx_sem);
}

