/* ---------------------------------------------------------------------------- */
/*
 * usart_simple.c
 *
 *  Created on: 2009-10-13
 *      Author: lucck
 */
/* ---------------------------------------------------------------------------- */
#include <stm32f10x_lib.h>
#include "system.h"
#include "usart_simple.h"
#include "config.h"
/* ---------------------------------------------------------------------------- */

#define USART1_TX_BIT 9
#define USART1_RX_BIT 10
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

/* ---------------------------------------------------------------------------- */
#define USED_UART CFG_USART2
#define USART_ALTERNATE_OUTPUT
/* ---------------------------------------------------------------------------- */
#define CFG_USART1 1
#define CFG_USART2 2

#if USED_UART==CFG_USART1
#define USART USART1
#define CLK_HZ CONFIG_PCLK2_HZ
#else
#define USART USART2
#define USART_PCLK_HZ CONFIG_PCLK1_HZ
#endif

/* ---------------------------------------------------------------------------- */
void usartsimple_init(unsigned baudrate)
{
#if USED_UART==CFG_USART1
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1;
        //Configure GPIO port TxD and RxD
        io_config(GPIOA,USART1_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
        io_config(GPIOA,USART1_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
#else /*USED_UART*/
#ifndef USART_ALTERNATE_OUTPUT
        RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
        RCC->APB1ENR |= RCC_APB1Periph_USART2;
        //Configure GPIO port TxD and RxD
        io_config(GPIOA,USART2_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
        io_config(GPIOA,USART2_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
#else /*USART_ALTERNATE_OUTPUT */
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |RCC_APB2Periph_GPIOD;
        RCC->APB1ENR |= RCC_APB1Periph_USART2;
        io_config(GPIOD,USART2_ALT_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
        io_config(GPIOD,USART2_ALT_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
        AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
#endif /*USART_ALTERNATE_OUTPUT */
#endif  /*USED_UART*/
    USART->CR1 &= ~(USART_M_BIT | USART_PCE_BIT);
    //Enable UART
    USART->CR1 = CR1_UE_SET;
    //Calculate baud rate
    uint32_t int_part = ((0x19 * USART_PCLK_HZ) / (0x04 * baudrate));
    uint32_t tmp = (int_part / 0x64) << 0x04;
    uint32_t fract_part = int_part - (0x64 * (tmp >> 0x04));
    tmp |= ((((fract_part * 0x10) + 0x32) / 0x64)) & ((u8)0x0F);
    USART->BRR = tmp;
    //Parity none
    USART->CR1 &= ~(USART_M_BIT | USART_PCE_BIT);
    //One stop bit
    USART->CR2 = USART_StopBits_1;
    //Enable receiver and transmitter and anable related interrupts
    USART->CR1 |=  USART_Mode_Tx | USART_Mode_Rx;

}

/* ---------------------------------------------------------------------------- */
//USART putchar
int usartsimple_putc(int ch, void* p)
{
    while((USART->SR & USART_TXE)==0);
    USART->DR = ch & 0xff;
    return ch;
}
/* ---------------------------------------------------------------------------- */
//Usart get char
int usartsimple_getc(void)
{
	if(USART->SR & USART_RXNE)
		return USART->DR & 0xff;
	else
		return USARTSIMPLE_EOF;
}
/* ---------------------------------------------------------------------------- */
//Check if char is available
int usartsimple_isc(void)
{
	return USART->SR & USART_RXNE;
}
/* ---------------------------------------------------------------------------- */
void usartsimple_puts(const char * str)
{
	while(*str)
		usartsimple_putc(*str++,0);
}

