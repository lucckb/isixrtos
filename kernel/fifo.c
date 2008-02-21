#include <isix/config.h>
#include <isix/types.h>
#include <isix/memory.h>
#include <isix/semaphore.h>
#include <isix/fifo.h>

#ifndef ISIX_DEBUG_FIFO
#define ISIX_DEBUG_FIFO ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_FIFO == ISIX_DBG_ON
#include <isix/printk.h>
#else
#define printk(...)
#endif


/*-------------------------------------------------------*/
/* Create queue for n elements
 * if succes return queue pointer else return null   */
fifo_t* fifo_create_isr(int n_elem, int elem_size,s8 interrupt)
{
   //Create fifo struct
   fifo_t *fifo = (fifo_t*)kmalloc(sizeof(fifo_t));
   if(!fifo)
   {
       printk("FifoCreate: Error alloc fifo struct\n");
       return NULL;
   }
   //Calculate size
   fifo->size = n_elem * elem_size;
   //Create used memory struct
   fifo->mem_p = (char*)kmalloc(fifo->size);
   if(!fifo->mem_p)
   {
     printk("FifoCreate: Error alloc data\n");
     kfree(fifo);
     return NULL;
   }
   //Fill element of structure
   fifo->elem_size = elem_size;
   fifo->rx_p = fifo->tx_p = fifo->mem_p;
   //Create RX sem as 0 element in fifo (task sleep)
   fifo->rx_sem = sem_create_isr(NULL,0,interrupt);
   if(!fifo->rx_sem)
   {
     printk("FifoCreate: Create sem RX failed\n");
     kfree(fifo->mem_p);
     kfree(fifo);
     return NULL;
   }
   //Create tx sem as numer of element in fifo
   fifo->tx_sem = sem_create_isr(NULL,n_elem,interrupt);
   if(!fifo->tx_sem)
   {
      printk("FifoCreate: Create sem TX failed\n");
      kfree(fifo->mem_p);
      sem_destroy(fifo->rx_sem);
      kfree(fifo);
      return NULL;
   }
   if(interrupt>=0) fifo->intmask = _BV(interrupt);
   else fifo->intmask = 0;
   printk("FifoCreate New fifo handler %08x\n",fifo);
   return fifo;
}

/*----------------------------------------------------------------*/
//Fifo send to other task
int fifo_write(fifo_t *fifo,const void *item,unsigned long timeout)
{
    if(!fifo) return ISIX_EINVARG;
    if(sem_wait(fifo->tx_sem,timeout)<0)
    {
        printk("FifoWrite: Timeout on TX queue\n");
        return ISIX_ETIMEOUT;
    }
    sched_lock_interrupt(fifo->intmask);
    memcpy(fifo->tx_p,item,fifo->elem_size);
    printk("FifoWrite: Data write at TXp %08x\n",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    sched_unlock_interrupt(fifo->intmask);
    printk("FifoWrite: New TXp %08x\n",fifo->tx_p);
    //Signaling RX thread with new data
    return sem_signal(fifo->rx_sem);
}
/*----------------------------------------------------------------*/
//Fifo send to other task
int fifo_write_isr(fifo_t *fifo,const void *item)
{
    if(!fifo) return ISIX_EINVARG;
    if(sem_get_isr(fifo->tx_sem)<0)
    {
        printk("FifoWriteISR: No space in TX queue\n");
        return ISIX_EFIFOFULL;
    }
    sched_lock();
    memcpy(fifo->tx_p,item,fifo->elem_size);
    printk("FifoWriteISR: Data write at TXp %08x\n",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    sched_unlock();
    printk("FifoWriteISR: New TXp %08x\n",fifo->tx_p);
    //Signaling RX thread with new data
    return sem_signal_isr(fifo->rx_sem);
}
/*----------------------------------------------------------------*/
//Fifo receive from other task
int fifo_read(fifo_t *fifo,void *item,unsigned long timeout)
{
    if(!fifo) return ISIX_EINVARG;
    if(sem_wait(fifo->rx_sem,timeout)<0)
    {
       printk("FifoRead: Timeout on RX queue\n");
       return ISIX_ETIMEOUT;
    }
    sched_lock_interrupt(fifo->intmask);
    memcpy(item,fifo->rx_p,fifo->elem_size);
    printk("FifoRead: Data write at RXp %08x\n",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    sched_unlock_interrupt(fifo->intmask);
    printk("FifoRead: New Rxp %08x\n",fifo->rx_p);
    //Signaling TX for space avail
    return sem_signal(fifo->tx_sem);
}

/*----------------------------------------------------------------*/
//Fifo receive from other task
int fifo_read_isr(fifo_t *fifo,void *item)
{
    if(!fifo) return ISIX_EINVARG;
    if(sem_get_isr(fifo->rx_sem)<0)
    {
       printk("FifoReadISR: No space in RX queue\n");
       return ISIX_EFIFOFULL;
    }
    sched_lock();
    memcpy(item,fifo->rx_p,fifo->elem_size);
    printk("FifoReadISR: Data write at RXp %08x\n",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    sched_unlock();
    printk("FifoReadISR: New Rxp %08x\n",fifo->rx_p);
    //Signaling TX for space avail
    return sem_signal_isr(fifo->tx_sem);
}

/*----------------------------------------------------------------*/
/* Delete created queue */
int fifo_destroy(fifo_t *fifo)
{
    sched_lock();
    //Check for TXSEM ban be destroyed
    if(__sem_can_destroy(fifo->tx_sem)==false)
    {
        printk("FifoDestroy: Error TXSem busy\n");
        sched_unlock();
        return ISIX_EBUSY;
    }
    //Check for RXSEM can be destroyed
    if(__sem_can_destroy(fifo->rx_sem)==false)
    {
        printk("FifoDestroy: Error RXSem busy\n");
        sched_unlock();
        return ISIX_EBUSY;
    }
    //Destroy RXSEM and TXSEM
    sem_destroy(fifo->rx_sem);
    sem_destroy(fifo->tx_sem);
    //Free queue used memory
    kfree(fifo->mem_p);
    kfree(fifo);
    sched_unlock();
    return ISIX_EOK;
}

/*----------------------------------------------------------------*/
//How many element is in fifo
int fifo_count(fifo_t *fifo)
{
    if(!fifo) return ISIX_EINVARG;
    return sem_getval(fifo->rx_sem);
}

/*----------------------------------------------------------------*/

