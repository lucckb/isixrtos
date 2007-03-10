#include <isix/device.h>
#include <isix/semaphore.h>
#include <isix/fifo.h>
#include <isix/memory.h>

#define DEBUG

#ifdef DEBUG
#include <isix/printk.h>
#else
#define printk(...)
#endif
/*------------------------------------------------------*/
typedef struct uart_prv_struct
{
    unsigned long base;
    fifo_t *rx_fifo,*tx_fifo;
}
uart_prv_t;

/*------------------------------------------------------*/
#define DEVPRV(p) ((uart_prv_t*)p->devprv)


/*------------------------------------------------------*/
//Uart read function
static int uart_read(file_t *fd,void *buf,size_t count)
{
    printk("UartRead: Read called\n");
    return 0;
}

/*------------------------------------------------------*/
//Uart write function
static int uart_write(file_t *fd,void *buf,size_t count)
{
    printk("UartWrite: Write called\n");
    return 0;
}

/*------------------------------------------------------*/
//Create Uart device
device_t* create_uart_device(unsigned long base,const char *name)
{
    device_t *dev = alloc_struct_device();
    if(!dev) return NULL;
    dev->devprv = kmalloc(sizeof(uart_prv_t));
    if(!dev->devprv)
    {
        printk("CreateUartDev: Cannot allocate devprv\n");
        kfree(dev);
        return NULL;
    }
    DEVPRV(dev)->rx_fifo = fifo_create(128,1);
    if(!DEVPRV(dev)->rx_fifo)
    {
        printk("CreateUartDev: Create RX fifo failed\n");
        kfree(dev->devprv);
        kfree(dev);
        return NULL;
    }
    DEVPRV(dev)->tx_fifo = fifo_create(128,1);
    if(!DEVPRV(dev)->tx_fifo)
    {
        printk("CreateUartDev: Create TX fifo failed\n");
        fifo_destroy(DEVPRV(dev)->rx_fifo);
        kfree(dev->devprv);
        kfree(dev);
        return NULL;
    }
    //Assign base pointer
    DEVPRV(dev)->base = base;
    dev->read = uart_read;
    dev->write = uart_write;
    //Create device name
    char *n1 = dev->name;
    while(*name) *n1++ = *name++;
    return dev;
}

/*------------------------------------------------------*/
//Destroy Uart device
void destroy_uart_device(device_t *dev)
{
    fifo_destroy(DEVPRV(dev)->tx_fifo);
    fifo_destroy(DEVPRV(dev)->rx_fifo);
    kfree(dev->devprv);
    kfree(dev);
}

/*------------------------------------------------------*/

