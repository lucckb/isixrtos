#include <isix/config.h>
#include <isix/types.h>
#include <isix/memory.h>
#include <isix/semaphore.h>
#include <isix/fifo.h>

#define DEBUG

#ifndef DEBUG
#define printk(...)
#else
#include <isix/printk.h>
#endif


/*-------------------------------------------------------*/
/* Create queue for n elements
 * if succes return queue pointer else return null   */
fifo_t* fifo_create(int n_elem, int elem_size)
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
   fifo->rx_sem = sem_create(NULL,0);
   if(!fifo->rx_sem)
   {
     printk("FifoCreate: Create sem RX failed\n");
     kfree(fifo->mem_p);
     kfree(fifo);
     return NULL;
   }
   //Create tx sem as numer of element in fifo
   fifo->tx_sem = sem_create(NULL,n_elem);
   if(!fifo->tx_sem)
   {
      printk("FifoCreate: Create sem TX failed\n");
      kfree(fifo->mem_p);
      sem_destroy(fifo->rx_sem);
      kfree(fifo);
      return NULL;
   }
   printk("FifoCreate New fifo handler %08x\n",fifo);
   return fifo;
}

/*----------------------------------------------------------------*/
//Fifo send to other task
int fifo_write(fifo_t *fifo,const void *item,unsigned long timeout)
{
    if(!fifo) return -1;
    if(sem_wait(fifo->tx_sem,timeout)<0)
    {
        printk("FifoWrite: Timeout on TX queue\n");
        return -1;
    }
    sched_lock();
    copy_memory(fifo->tx_p,item,fifo->elem_size);
    printk("FifoWrite: Data write at TXp %08x\n",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    sched_unlock();
    printk("FifoWrite: New TXp %08x\n",fifo->tx_p);
    //Signaling RX thread with new data
    if(sem_signal(fifo->rx_sem)<0) return -1;
    else return 0;
}
/*----------------------------------------------------------------*/
//Fifo send to other task
int fifo_write_isr(fifo_t *fifo,const void *item)
{
    if(!fifo) return -1;
    if(sem_get_isr(fifo->tx_sem)<0)
    {
        printk("FifoWriteISR: No space in TX queue\n");
        return -1;
    }
    sched_lock();
    copy_memory(fifo->tx_p,item,fifo->elem_size);
    printk("FifoWriteISR: Data write at TXp %08x\n",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    sched_unlock();
    printk("FifoWriteISR: New TXp %08x\n",fifo->tx_p);
    //Signaling RX thread with new data
    if(sem_signal_isr(fifo->rx_sem)<0) return -1;
    else return 0;
}
/*----------------------------------------------------------------*/
//Fifo receive from other task
int fifo_read(fifo_t *fifo,void *item,unsigned long timeout)
{
    if(!fifo) return -1;
    if(sem_wait(fifo->rx_sem,timeout)<0)
    {
       printk("FifoRead: Timeout on RX queue\n");
       return -1;
    }
    sched_lock();
    copy_memory(item,fifo->rx_p,fifo->elem_size);
    printk("FifoRead: Data write at RXp %08x\n",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    sched_unlock();
    printk("FifoRead: New Rxp %08x\n",fifo->rx_p);
    //Signaling TX for space avail
    if(sem_signal(fifo->tx_sem)<0) return -1;
    else return 0;
}

/*----------------------------------------------------------------*/
//Fifo receive from other task
int fifo_read_isr(fifo_t *fifo,void *item)
{
    if(!fifo) return -1;
    if(sem_get_isr(fifo->rx_sem)<0)
    {
       printk("FifoReadISR: No space in RX queue\n");
       return -1;
    }
    sched_lock();
    copy_memory(item,fifo->rx_p,fifo->elem_size);
    printk("FifoReadISR: Data write at RXp %08x\n",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    sched_unlock();
    printk("FifoReadISR: New Rxp %08x\n",fifo->rx_p);
    //Signaling TX for space avail
    if(sem_signal_isr(fifo->tx_sem)<0) return -1;
    else return 0;
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
        return -1;
    }
    //Check for RXSEM can be destroyed
    if(__sem_can_destroy(fifo->rx_sem)==false)
    {
        printk("FifoDestroy: Error RXSem busy\n");
        sched_unlock();
        return -1;
    }
    //Destroy RXSEM and TXSEM
    sem_destroy(fifo->rx_sem);
    sem_destroy(fifo->tx_sem);
    //Free queue used memory
    kfree(fifo->mem_p);
    kfree(fifo);
    sched_unlock();
    return 0;
}

/*----------------------------------------------------------------*/
