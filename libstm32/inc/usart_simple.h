
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



#define USARTSIMPLE_FL_ALTERNATE 1U
#define USARTSIMPLE_FL_ALTERNATE_EXT 2U
#define USARTSIMPLE_FL_ALTERNATE_PORT(x) ( ((x)<<2U)|USARTSIMPLE_FL_ALTERNATE_EXT )
#define USARTSIMPLE_FL_ALTERNATE_PA USARTSIMPLE_FL_ALTERNATE_PORT(0U)
#define USARTSIMPLE_FL_ALTERNATE_PB USARTSIMPLE_FL_ALTERNATE_PORT(1U)
#define USARTSIMPLE_FL_ALTERNATE_PC USARTSIMPLE_FL_ALTERNATE_PORT(2U)
#define USARTSIMPLE_FL_ALTERNATE_PD USARTSIMPLE_FL_ALTERNATE_PORT(3U)
#define USARTSIMPLE_FL_ALTERNATE_PE USARTSIMPLE_FL_ALTERNATE_PORT(4U)
#define USARTSIMPLE_FL_ALTERNATE_PF USARTSIMPLE_FL_ALTERNATE_PORT(5U)
#define USARTSIMPLE_FL_ALTERNATE_PG USARTSIMPLE_FL_ALTERNATE_PORT(6U)
#define USARTSIMPLE_FL_ALTERNATE_PH USARTSIMPLE_FL_ALTERNATE_PORT(7U)
#define USARTSIMPLE_FL_ALTERNATE_PI USARTSIMPLE_FL_ALTERNATE_PORT(8U)


#define USARTSIMPLE_FL_NORX  0x80000000U



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
