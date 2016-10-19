
/*
 * usart_simple.h
 *
 *  Created on: 2009-10-13
 *      Author: lucck
 */


#ifndef USART_SIMPLE_H_
#define USART_SIMPLE_H_

#include <stm32lib.h>

#ifdef __cplusplus
 namespace stm32 {
	extern "C" {
#endif


enum usartsimple_errno
{
	USARTSIMPLE_INIT_OK,
	USARTSIMPLE_EOF = -1,
	USARTSIMPLE_NOT_INIT = -2,
	USARTSIMPLE_INIT_FAIL=-3
};

enum usartsimple_flags
{
	USARTSIMPLE_FL_ALTERNATE = 0x1U,
	USARTSIMPLE_FL_NORX =	   0x80000000U
};

//Initialize uart
int usartsimple_init(USART_TypeDef *usart, unsigned baudrate, unsigned flags,
		unsigned long pclk1_hz, unsigned long pclk2_hz);


//USART putchar
int usartsimple_putc(int ch,void *);


//Usart get char
int usartsimple_getc(void);


//Check if char is available
int usartsimple_isc(void);

//Usart put string
void usartsimple_puts(const char * str);



#ifdef __cplusplus
}
}
#endif



#endif /* USART_SIMPLE_H_ */
