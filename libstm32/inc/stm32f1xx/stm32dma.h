/*
 * stm32dma.h
 *
 *  Created on: 09-03-2011
 *      Author: lucck
 */

#ifndef STM32F1DMA_H_
#define STM32F1DMA_H_
/* ---------------------------------------------------------------------------- */
#include <stddef.h>
#include <stdbool.h>
/* ---------------------------------------------------------------------------- */
#define DMA2_FLAG 0x10000000

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
static inline void dma_disable(enum dma_cntr ch)
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
static inline void dma_channel_config(DMA_Channel_TypeDef *DMAchx, uint32_t config,
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
  if ((flag & DMA2_FLAG) != 0)
  {
    /* Clear the selected DMA flags */
    DMA2->IFCR = flag & (~DMA2_FLAG);
  }
  else
  {
    /* Clear the selected DMA flags */
    DMA1->IFCR = flag & (~DMA2_FLAG);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Sets the number of data units in the current DMAy Channelx transfer.
  * @param  DMAy_Channelx: where y can be 1 or 2 to select the DMA and 
  *         x can be 1 to 7 for DMA1 and 1 to 5 for DMA2 to select the DMA Channel.
  * @param  DataNumber: The number of data units in the current DMAy Channelx
  *         transfer.   
  * @note   This function can only be used when the DMAy_Channelx is disabled.                 
  * @retval None.
  */
static inline void dma_set_curr_data_counter(DMA_Channel_TypeDef* DMAchx, uint16_t ncnt)
{
 
   /* Write to DMAy Channelx CNDTR */
   DMAchx->CNDTR = ncnt;  
}
/* ---------------------------------------------------------------------------- */
static inline void dma_set_memory_address(DMA_Channel_TypeDef* DMAchx, const void *mem )
{
	DMAchx->CMAR = (unsigned long)mem;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Returns the number of remaining data units in the current
  *         DMAy Channelx transfer.
  * @param  DMAy_Channelx: where y can be 1 or 2 to select the DMA and 
  *   x can be 1 to 7 for DMA1 and 1 to 5 for DMA2 to select the DMA Channel.
  * @retval The number of remaining data units in the current DMAy Channelx
  *         transfer.
  */
static inline uint16_t dma_get_curr_data_counter(DMA_Channel_TypeDef* DMAchx)
{
  /* Return the number of remaining data units for DMAy Channelx */
  return DMAchx->CNDTR;
}

/* ---------------------------------------------------------------------------- */
static inline bool dma_get_flag_status(uint32_t DMAy_FLAG)
{
  uint32_t tmpreg;
  
  /* Calculate the used DMAy */
  if (DMAy_FLAG & DMA2_FLAG )
  {
    /* Get DMA2 ISR register value */
    tmpreg = DMA2->ISR;
  }
  else
  {
    /* Get DMA1 ISR register value */
    tmpreg = DMA1->ISR;
  }
  /* Check the status of the specified DMAy flag */
  return(tmpreg & DMAy_FLAG & ~(DMA2_FLAG))?(true):(false);
}
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
/* ---------------------------------------------------------------------------- */

#undef DMA2_FLAG 
#endif /* STM32DMA_H_ */
