/*
 * stm32dma.h
 *
 *  Created on: 09-03-2011
 *      Author: lucck
 */

#ifndef STM32DMA_H_
#define STM32DMA_H_
/* ---------------------------------------------------------------------------- */
#include <stm32f10x_lib.h>
#include <cstddef>
/* ---------------------------------------------------------------------------- */

#ifdef __cplusplus
 namespace stm32 {
#endif

/* ---------------------------------------------------------------------------- */
enum dma_cntr
{
	DMACNTR_1,
	DMACNTR_2
};


/* ---------------------------------------------------------------------------- */
//Enable the selected dma controller
static inline void dma_enable(enum dma_cntr ch)
{
	if(ch == DMACNTR_1)
	{
		RCC->AHBENR |= RCC_AHBENR_DMA1EN;
		DMA1->IFCR = 0xFFFFFFFF;
	}
#if defined(STM32F10X_HD) || defined(STM32F10X_CL)
	else if(ch == DMACNTR_2)
	{
		RCC->AHBENR |= RCC_AHBENR_DMA2EN;
		DMA2->IFCR = 0xFFFFFFFF;
	}
#endif
}
/* ---------------------------------------------------------------------------- */
//Disable the dma channel
void dma_disable(enum dma_cntr ch)
{
	if(ch == DMACNTR_1)
	{
		RCC->AHBENR &= ~RCC_AHBENR_DMA1EN;
	}
#if defined(STM32F10X_HD) || defined(STM32F10X_CL)
	else if(ch == DMACNTR_2)
	{
		RCC->AHBENR &= ~RCC_AHBENR_DMA2EN;
	}
#endif
}
/* ---------------------------------------------------------------------------- */
//Dma channel enable
static inline void dma_channel_enable(DMA_Channel_TypeDef *DMAchx, uint32_t config,
		volatile void * mem, volatile void *periph, size_t size)
{
	DMAchx->CCR = config;
	DMAchx->CPAR = (unsigned long)periph;
	DMAchx->CMAR = (unsigned long)mem;
	DMAchx->CNDTR = size;
}
/* ---------------------------------------------------------------------------- */
static inline void dma_channel_enable(DMA_Channel_TypeDef *DMAchx)
{
	DMAchx->CCR |= 0x1;
}
/* ---------------------------------------------------------------------------- */
static inline void dma_channel_disable(DMA_Channel_TypeDef *DMAchx)
{
	DMAchx->CCR &= ~0x1;
}
/* ---------------------------------------------------------------------------- */
static inline void dma_irq_enable( DMA_Channel_TypeDef* DMAchx, unsigned it)
{
	DMAchx->CCR |= it;
}
/* ---------------------------------------------------------------------------- */
static inline void dma_irq_disable( DMA_Channel_TypeDef* DMAchx, unsigned it)
{
	DMAchx->CCR &= ~it;
}
/* ---------------------------------------------------------------------------- */
static inline void dma_clear_flag(uint32_t flag)
{
  if ((flag & 0x10000000) != 0)
  {
    /* Clear the selected DMA flags */
    DMA2->IFCR = flag;
  }
  else
  {
    /* Clear the selected DMA flags */
    DMA1->IFCR = flag;
  }
}
/* ---------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif
/* ---------------------------------------------------------------------------- */
#endif /* STM32DMA_H_ */
