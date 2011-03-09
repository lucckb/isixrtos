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

/*----------------------------------------------------------*/
#ifdef __cplusplus
 namespace stm32 {
	extern "C" {
#endif

/* ---------------------------------------------------------------------------- */
enum
{
	USARTSIMPLE_INIT_OK,
	USARTSIMPLE_EOF = -1,
	USARTSIMPLE_NOT_INIT = -2,
	USARTSIMPLE_INIT_FAIL=-3
};
/* ---------------------------------------------------------------------------- */
//Initialize uart
int usartsimple_init(USART_TypeDef *usart, unsigned baudrate, bool alternate,
		unsigned long pclk1_hz, unsigned long pclk2_hz);

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
}
#endif

/* ---------------------------------------------------------------------------- */

#endif /* USART_SIMPLE_H_ */
