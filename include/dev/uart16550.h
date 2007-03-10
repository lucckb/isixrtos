#ifndef __DEV_UART16550_H
#define __DEV_UART16550_H


//Uart0 Base register
#define UART0_BASE 0xE000C000

//Uart1 Base register
#define UART1_BASE 0xE0010000

/*------------------------------------------------------*/
//Create Uart device
device_t* create_uart_device(unsigned long base,const char *name);

/*------------------------------------------------------*/
//Destroy Uart device
void destroy_uart_device(device_t *dev);

/*------------------------------------------------------*/

#endif
