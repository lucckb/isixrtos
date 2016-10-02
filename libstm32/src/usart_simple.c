
/*
 * usart_simple.c
 *
 *  Created on: 2009-10-13
 *      Author: lucck
 */

#include <stm32lib.h>
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
#define CR1_UE_SET 0x2000
#define CCR_ENABLE_SET 0x00000001
#define USART1_DR_BASE  0x40013804
#define USART_M_BIT (1<<12)
#define USART_PCE_BIT (1<<10)
#define USART_PS_BIT (1<<9)
#define USART_TXEIE (1<<7)
#define USART_RXNEIE (1<<5)
#define USART_RXNE (1<<5)
#define USART_TXE (1<<7)
#define USART_ORE (1<<3)
#define USART_TC (1<<6)
#define USART_FE (1<<1)
#define USART_PE (1<<0)
#define USART_ORE (1<<3)




static USART_TypeDef *usart;


int usartsimple_init(USART_TypeDef *usart_, unsigned baudrate, unsigned flags,
		unsigned long pclk1_hz, unsigned long pclk2_hz)
{
	if(usart_==USART1)
	{
		RCC->APB2ENR |= RCC_APB2Periph_USART1;
		if(!(flags & USARTSIMPLE_FL_ALTERNATE))
		{
			gpio_clock_enable( GPIOA, true );
			//Configure GPIO port TxD and RxD
			gpio_abstract_config(GPIOA,USART1_TX_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			if(!(flags & USARTSIMPLE_FL_NORX ) )
				gpio_abstract_config(GPIOA,USART1_RX_BIT, AGPIO_MODE_INPUT_FLOATING, 0 );
#if defined(STM32MCU_MAJOR_TYPE_F2) || 	defined(STM32MCU_MAJOR_TYPE_F4)
			gpio_pin_AF_config(GPIOA, USART1_TX_BIT , GPIO_AF_USART1 );
			if(!(flags & USARTSIMPLE_FL_NORX ) )
				gpio_pin_AF_config(GPIOA, USART1_RX_BIT , GPIO_AF_USART1 );
#endif
		}
		else
		{
			gpio_clock_enable( GPIOB, true );
			//Configure GPIO port TxD and RxD
			gpio_abstract_config(GPIOB,USART1_ALT_TX_BIT, AGPIO_MODE_ALTERNATE_PP,AGPIO_SPEED_HALF );

			if(!(flags & USARTSIMPLE_FL_NORX ) )
				gpio_abstract_config(GPIOB,USART1_ALT_RX_BIT, AGPIO_MODE_INPUT_FLOATING, 0 );
#if defined(STM32MCU_MAJOR_TYPE_F2) || 	defined(STM32MCU_MAJOR_TYPE_F4)
			gpio_pin_AF_config(GPIOB, USART1_ALT_TX_BIT , GPIO_AF_USART1 );

			if(!(flags & USARTSIMPLE_FL_NORX ) )
				gpio_pin_AF_config(GPIOB, USART1_ALT_RX_BIT , GPIO_AF_USART1 );
#else
			RCC->APB2ENR |= RCC_APB2Periph_AFIO;
			AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;
#endif
		}
	}
	else if(usart_==USART2)
	{
		if(!(flags & USARTSIMPLE_FL_ALTERNATE))
		{
			gpio_clock_enable( GPIOA, true );
			RCC->APB1ENR |= RCC_APB1Periph_USART2;
			//Configure GPIO port TxD and RxD
			gpio_abstract_config( GPIOA,USART2_TX_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			gpio_abstract_config( GPIOA,USART2_RX_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
		}
		else
		{
			gpio_clock_enable( GPIOD, true );
#if defined(STM32MCU_MAJOR_TYPE_F1)
			RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
#endif
			RCC->APB1ENR |= RCC_APB1Periph_USART2;
			gpio_abstract_config(GPIOD,USART2_ALT_TX_BIT,AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			if(!(flags & USARTSIMPLE_FL_NORX ) )
			gpio_abstract_config(GPIOD,USART2_ALT_RX_BIT, AGPIO_MODE_INPUT_FLOATING, 0);
#if defined(STM32MCU_MAJOR_TYPE_F1)
			AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
#elif defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
			gpio_pin_AF_config( GPIOD, USART2_ALT_TX_BIT, 7 );
			if(!(flags & USARTSIMPLE_FL_NORX ) )
			gpio_pin_AF_config( GPIOD, USART2_ALT_RX_BIT, 7 );
#endif
		}
	}
	else
	{
		return USARTSIMPLE_INIT_FAIL;
	}
	usart = usart_;
	usart->CR1 &= ~(USART_M_BIT | USART_PCE_BIT);
    //Enable UART
    usart->CR1 = CR1_UE_SET;
    unsigned long pclk_hz;
    if(usart==USART1) pclk_hz = pclk2_hz;
    else pclk_hz = pclk1_hz;
    //Calculate baud rate
    uint32_t int_part = ((0x19 * pclk_hz) / (0x04 * baudrate));
    uint32_t tmp = (int_part / 0x64) << 0x04;
    uint32_t fract_part = int_part - (0x64 * (tmp >> 0x04));
    tmp |= ((((fract_part * 0x10) + 0x32) / 0x64)) & ((u8)0x0F);
    usart->BRR = tmp;
    //Parity none
    usart->CR1 &= ~(USART_M_BIT | USART_PCE_BIT);
    //One stop bit
    usart->CR2 = USART_StopBits_1;
    //Enable receiver and transmitter and anable related interrupts
    usart->CR1 |=  USART_Mode_Tx | USART_Mode_Rx;
    return USARTSIMPLE_INIT_OK;
}


//USART putchar
int usartsimple_putc(int ch, void* p)
{
	(void)p;
	if(!usart) return USARTSIMPLE_NOT_INIT;
	while((usart->SR & USART_TXE)==0);
    usart->DR = ch & 0xff;
    return ch;
}

//Usart get char
int usartsimple_getc(void)
{
	if(!usart) return USARTSIMPLE_NOT_INIT;
	if(usart->SR & USART_RXNE)
		return usart->DR & 0xff;
	else
		return USARTSIMPLE_EOF;
}

//Check if char is available
int usartsimple_isc(void)
{
	if(!usart) return USARTSIMPLE_NOT_INIT;
	return usart->SR & USART_RXNE;
}

void usartsimple_puts(const char * str)
{
	while(*str)
		usartsimple_putc(*str++,0);
}



