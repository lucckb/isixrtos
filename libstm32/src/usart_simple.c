
/*
 * usart_simple.c
 *
 *  Created on: 2009-10-13
 *      Author: lucck
 */

#include <stm32lib.h>
#include <stm32usart.h>
#include "stm32system.h"
#include "usart_simple.h"



#define USART1_TX_BIT 9
#define USART1_RX_BIT 10
#define USART1_ALT_TX_BIT 6
#define USART1_ALT_RX_BIT 7
#define USART2_TX_BIT 2
#define USART2_ALT_TX_BIT 5
#define USART2_ALT_RX_BIT 6
#define USART2_RX_BIT 3


#if defined(STM32MCU_MAJOR_TYPE_F2) || defined(STM32MCU_MAJOR_TYPE_F4) \
|| defined(STM32MCU_MAJOR_TYPE_F37) || defined(STM32MCU_MAJOR_TYPE_F7) \
|| defined(STM32MCU_MAJOR_TYPE_F3)
#define IS_NEW_GPIO 1
#endif

static USART_TypeDef *usart;


int usartsimple_init(USART_TypeDef *usart_, unsigned baudrate, unsigned flags,
		unsigned long pclk1_hz, unsigned long pclk2_hz)
{
	if(usart_==USART1)
	{
#if defined(RCC_APB2Periph_USART1)
		RCC->APB2ENR |= RCC_APB2Periph_USART1;
#elif defined(RCC_APB2ENR_USART1EN)
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
#endif
		if(flags & USARTSIMPLE_FL_ALTERNATE)
		{
			gpio_clock_enable( GPIOB, true );
			//Configure GPIO port TxD and RxD
			gpio_abstract_config(GPIOB,USART1_ALT_TX_BIT, AGPIO_MODE_ALTERNATE_PP,AGPIO_SPEED_HALF );

			if(!(flags & USARTSIMPLE_FL_NORX ) )
				gpio_abstract_config(GPIOB,USART1_ALT_RX_BIT, AGPIO_MODE_INPUT_FLOATING, 0 );
#if IS_NEW_GPIO
			gpio_pin_AF_config(GPIOB, USART1_ALT_TX_BIT , GPIO_AF_7 );

			if(!(flags & USARTSIMPLE_FL_NORX ) )
				gpio_pin_AF_config(GPIOB, USART1_ALT_RX_BIT , GPIO_AF_7 );
#else
			RCC->APB2ENR |= RCC_APB2Periph_AFIO;
			AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;
#endif
		}
		else if( flags & USARTSIMPLE_FL_ALTERNATE_PC )
		{
#if IS_NEW_GPIO
			gpio_clock_enable( GPIOC, true );
			gpio_abstract_config( GPIOC, 4, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			gpio_pin_AF_config( GPIOC, 4, GPIO_AF_7 );

			if(!(flags & USARTSIMPLE_FL_NORX ) ) {
				gpio_abstract_config( GPIOC, 5, AGPIO_MODE_INPUT_FLOATING, 0 );
				gpio_pin_AF_config( GPIOC, 5, GPIO_AF_7 );
			}
#endif
		}
		else
		{
			gpio_clock_enable( GPIOA, true );
			//Configure GPIO port TxD and RxD
			gpio_abstract_config(GPIOA,USART1_TX_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			if(!(flags & USARTSIMPLE_FL_NORX ) )
				gpio_abstract_config(GPIOA,USART1_RX_BIT, AGPIO_MODE_INPUT_FLOATING, 0 );
#if IS_NEW_GPIO
 
			gpio_pin_AF_config(GPIOA, USART1_TX_BIT , GPIO_AF_7 );
			if(!(flags & USARTSIMPLE_FL_NORX ) )
				gpio_pin_AF_config(GPIOA, USART1_RX_BIT , GPIO_AF_7 );
#endif
		}
	}
	else if(usart_==USART2)
	{
#if defined(RCC_APB1Periph_USART2)
		RCC->APB1ENR |= RCC_APB1Periph_USART2;
#elif defined(RCC_APB1ENR_USART2EN)
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
#endif
		if( flags & USARTSIMPLE_FL_ALTERNATE )
		{
			gpio_clock_enable( GPIOD, true );
#if defined(STM32MCU_MAJOR_TYPE_F1)
			RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
#endif
			gpio_abstract_config( GPIOD, USART2_ALT_TX_BIT,AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			if(!(flags & USARTSIMPLE_FL_NORX ) )
			gpio_abstract_config( GPIOD, USART2_ALT_RX_BIT, AGPIO_MODE_INPUT_FLOATING, 0 );
#if defined(STM32MCU_MAJOR_TYPE_F1)
			AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
#elif IS_NEW_GPIO
			gpio_pin_AF_config( GPIOD, USART2_ALT_TX_BIT, GPIO_AF_7 );
			if(!(flags & USARTSIMPLE_FL_NORX ) )
			gpio_pin_AF_config( GPIOD, USART2_ALT_RX_BIT, GPIO_AF_7 );
#endif
		}
#ifdef IS_NEW_GPIO
		else if( flags & USARTSIMPLE_FL_ALTERNATE_PB ) {
			gpio_clock_enable( GPIOB, true );
			gpio_abstract_config( GPIOB, 3, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			gpio_pin_AF_config( GPIOB, 3, GPIO_AF_7 );
			if(!(flags & USARTSIMPLE_FL_NORX ) ) {
				gpio_abstract_config( GPIOB, 4, AGPIO_MODE_INPUT_FLOATING, 0 );
				gpio_pin_AF_config( GPIOB, 4, GPIO_AF_7 );
			}
		}
#endif
		else
		{
			//Default config
			gpio_clock_enable( GPIOA, true );
			//Configure GPIO port TxD and RxD
			gpio_abstract_config( GPIOA,USART2_TX_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			gpio_abstract_config( GPIOA,USART2_RX_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );

		}
	}
	else
	{
		return USARTSIMPLE_INIT_FAIL;
	}
	usart = usart_;
	usart_init( usart, baudrate, USART_WordLength_8b, USART_StopBits_1,
			    USART_Parity_No, USART_Mode_Rx|USART_Mode_Tx, 
				USART_HardwareFlowControl_None, pclk1_hz, pclk2_hz );
    usart_cmd( usart, true );
    return USARTSIMPLE_INIT_OK;
}


//USART putchar
int usartsimple_putc(int ch, void* p)
{
	(void)p;
	if(!usart) return USARTSIMPLE_NOT_INIT;
	while( !usart_get_flag_status(usart, USART_FLAG_TXE) );
    usart_send_data( usart, ch );
    return ch;
}

//Usart get char
int usartsimple_getc(void)
{
	if(!usart) return USARTSIMPLE_NOT_INIT;
	if( usart_get_flag_status( usart, USART_FLAG_RXNE ) )
		return usart_receive_data( usart );
	else
		return USARTSIMPLE_EOF;
}

//Check if char is available
int usartsimple_isc(void)
{
	if(!usart) return USARTSIMPLE_NOT_INIT;
	return usart_get_flag_status( usart, USART_FLAG_RXNE );
}

void usartsimple_puts(const char * str)
{
	while(*str)
		usartsimple_putc(*str++,0);
}



