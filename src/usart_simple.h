/* ---------------------------------------------------------------------------- */
/*
 * usart_simple.h
 *
 *  Created on: 2009-10-13
 *      Author: lucck
 */

/* ---------------------------------------------------------------------------- */
#ifndef USART_SIMPLE_H_
#define USART_SIMPLE_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* ---------------------------------------------------------------------------- */
#define USARTSIMPLE_EOF (-1)

/* ---------------------------------------------------------------------------- */
//Initialize uart
void usartsimple_init(unsigned baudrate);

/* ---------------------------------------------------------------------------- */
//USART putchar
int usartsimple_putc(int ch,void *);

/* ---------------------------------------------------------------------------- */
//Usart get char
int usartsimple_getc(void);

/* ---------------------------------------------------------------------------- */
//Check if char is available
int usartsimple_isc(void);
/* ---------------------------------------------------------------------------- */
//Usart put string
void usartsimple_puts(const char * str);

/* ---------------------------------------------------------------------------- */

#ifdef __cplusplus
 }
#endif

/* ---------------------------------------------------------------------------- */

#endif /* USART_SIMPLE_H_ */
