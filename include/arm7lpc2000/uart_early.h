#ifndef __ASM_UART_EARLY_H
#define __ASM_UART_EARLY_H


/*----------------------------------------------------*/

/* Inicjalizacja Uart0 */
void uart_early_init(void);

/*----------------------------------------------------*/

//Nadawanie znak
void uart_early_putchar(char c);

/*----------------------------------------------------*/


#endif
