#ifndef __ISIX_PRINTK_H
#define __ISIX_PRINTK_H

void printk_init(unsigned short baud_rate);

void printk(const char *text,...);

//Wyznaczenie predkosci
#define UART_BAUD(baud) (unsigned short)(PCLK/(baud*16.0) + 0.5)
#define PCLK 60000000

#endif


