#include <isix/config.h>
#include <isix/types.h>
#include <isix/memory.h>
#include <isix/semaphore.h>
#include <isix/fifo.h>
#include <prv/semaphore.h>
#include <prv/scheduler.h>
#include <string.h>

#ifndef ISIX_DEBUG_FIFO
#define ISIX_DEBUG_FIFO ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_FIFO == ISIX_DBG_ON
#include <isix/printk.h>
#else
#define printk(...)
#endif


/*-------------------------------------------------------*/
/* Queue structure */
struct fifo_struct
{
    char *rx_p;     //Pointer to rx
    char *tx_p;     //Pointer to tx
    char *mem_p;    //Pointer to allocated memory
    int size;       //Total fifo size
    int elem_size; //Element count
    sem_t *rx_sem;  //Semaphore rx
    sem_t *tx_sem;  //Semaphore for tx
};

/*-------------------------------------------------------*/

/* Create queue for n elements
 * if succes return queue pointer else return null   */
fifo_t* isix_fifo_create(int n_elem, int elem_size)
{
   //Create fifo struct
   fifo_t *fifo = (fifo_t*)isix_alloc(sizeof(fifo_t));
   if(!fifo)
   {
       printk("FifoCreate: Error alloc fifo struct\n");
       return NULL;
   }
   //Calculate size
   fifo->size = n_elem * elem_size;
   //Create used memory struct
   fifo->mem_p = (char*)isix_alloc(fifo->size);
   if(!fifo->mem_p)
   {
     printk("FifoCreate: Error alloc data\n");
     isix_free(fifo);
     return NULL;
   }
   //Fill element of structure
   fifo->elem_size = elem_size;
   fifo->rx_p = fifo->tx_p = fifo->mem_p;
   //Create RX sem as 0 element in fifo (task sleep)
   fifo->rx_sem = isix_sem_create(NULL,0);
   if(!fifo->rx_sem)
   {
     printk("FifoCreate: Create sem RX failed\n");
     isix_free(fifo->mem_p);
     isix_free(fifo);
     return NULL;
   }
   //Create tx sem as numer of element in fifo
   fifo->tx_sem = isix_sem_create(NULL,n_elem);
   if(!fifo->tx_sem)
   {
      printk("FifoCreate: Create sem TX failed\n");
      isix_free(fifo->mem_p);
      isix_sem_destroy(fifo->rx_sem);
      isix_free(fifo);
      return NULL;
   }
   printk("FifoCreate New fifo handler %08x\n",fifo);
   return fifo;
}

/*----------------------------------------------------------------*/
//Fifo send to other task
int isix_fifo_write(fifo_t *fifo,const void *item, tick_t timeout)
{
    if(!fifo) return ISIX_EINVARG;
    //FIXME tu sie zesralo
    if(isix_sem_wait(fifo->tx_sem,timeout)<0)
    {
        printk("FifoWrite: Timeout on TX queue\n");
        return ISIX_ETIMEOUT;
    }
    isixp_enter_critical();
    memcpy(fifo->tx_p,item,fifo->elem_size);
    printk("FifoWrite: Data write at TXp %08x\n",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    isixp_exit_critical();
    printk("FifoWrite: New TXp %08x\n",fifo->tx_p);
    //Signaling RX thread with new data
    return isix_sem_signal(fifo->rx_sem);
}
/*----------------------------------------------------------------*/
//Fifo send to other task
int isix_fifo_write_isr(fifo_t *fifo,const void *item)
{
    if(!fifo) return ISIX_EINVARG;
    if(isix_sem_get_isr(fifo->tx_sem)<0)
    {
        printk("FifoWriteISR: No space in TX queue\n");
        return ISIX_EFIFOFULL;
    }
    isixp_enter_critical();
    memcpy(fifo->tx_p,item,fifo->elem_size);
    printk("FifoWriteISR: Data write at TXp %08x\n",fifo->tx_p);
    fifo->tx_p+= fifo->elem_size;
    if(fifo->tx_p >= fifo->mem_p+fifo->size) fifo->tx_p = fifo->mem_p;
    isixp_exit_critical();
    printk("FifoWriteISR: New TXp %08x\n",fifo->tx_p);
    //Signaling RX thread with new data
    return isix_sem_signal_isr(fifo->rx_sem);
}
/*----------------------------------------------------------------*/
//Fifo receive from other task
int isix_fifo_read(fifo_t *fifo,void *item, tick_t timeout)
{
    if(!fifo) return ISIX_EINVARG;
    if(isix_sem_wait(fifo->rx_sem,timeout)<0)
    {
       printk("FifoRead: Timeout on RX queue\n");
       return ISIX_ETIMEOUT;
    }
    isixp_enter_critical();
    memcpy(item,fifo->rx_p,fifo->elem_size);
    printk("FifoRead: Data write at RXp %08x\n",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    isixp_exit_critical();
    printk("FifoRead: New Rxp %08x\n",fifo->rx_p);
    //Signaling TX for space avail
    return isix_sem_signal(fifo->tx_sem);
}

/*----------------------------------------------------------------*/
//Fifo receive from other task
int isix_fifo_read_isr(fifo_t *fifo,void *item)
{
    if(!fifo) return ISIX_EINVARG;
    if(isix_sem_get_isr(fifo->rx_sem)<0)
    {
       printk("FifoReadISR: No space in RX queue\n");
       return ISIX_EFIFOFULL;
    }
    isixp_enter_critical();
    memcpy(item,fifo->rx_p,fifo->elem_size);
    printk("FifoReadISR: Data write at RXp %08x\n",fifo->rx_p);
    fifo->rx_p+= fifo->elem_size;
    if(fifo->rx_p >= fifo->mem_p+fifo->size) fifo->rx_p = fifo->mem_p;
    isixp_exit_critical();
    printk("FifoReadISR: New Rxp %08x\n",fifo->rx_p);
    //Signaling TX for space avail
    return isix_sem_signal_isr(fifo->tx_sem);
}

/*----------------------------------------------------------------*/
/* Delete created queue */
int isix_fifo_destroy(fifo_t *fifo)
{
    isixp_enter_critical();
    //Check for TXSEM ban be destroyed
    if(isixp_sem_can_destroy(fifo->tx_sem)==false)
    {
        printk("FifoDestroy: Error TXSem busy\n");
        isixp_exit_critical();
        return ISIX_EBUSY;
    }
    //Check for RXSEM can be destroyed
    if(isixp_sem_can_destroy(fifo->rx_sem)==false)
    {
        printk("FifoDestroy: Error RXSem busy\n");
        isixp_exit_critical();
        return ISIX_EBUSY;
    }
    //Destroy RXSEM and TXSEM
    isix_sem_destroy(fifo->rx_sem);
    isix_sem_destroy(fifo->tx_sem);
    //Free queue used memory
    isix_free(fifo->mem_p);
    isix_free(fifo);
    isixp_exit_critical();
    return ISIX_EOK;
}

/*----------------------------------------------------------------*/
//How many element is in fifo
int isix_fifo_count(fifo_t *fifo)
{
    if(!fifo) return ISIX_EINVARG;
    return isix_sem_getval(fifo->rx_sem);
}

/*----------------------------------------------------------------*/

