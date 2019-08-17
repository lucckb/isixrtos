/*
 * =====================================================================================
 *
 *       Filename:  stm32dac.h
 *
 *    Description: STM32 DAC standard perhipheral library 
 *
 *        Version:  1.0
 *        Created:  25.02.2014 21:01:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

  
#ifndef  stm32dac_H_
#define  stm32dac_H_
  
#include <stdbool.h>
#include <stdint.h>
#include "stm32fxxx_dac.h"
#include "stm32lib.h"
#include "stm32rcc.h"
  
//Local defs
/* CR register Mask */
#define CR_CLEAR_MASK              ((uint32_t)0x00000FFE)

/* DAC Dual Channels SWTRIG masks */
#define DUAL_SWTRIG_SET            ((uint32_t)0x00000003)
#define DUAL_SWTRIG_RESET          ((uint32_t)0xFFFFFFFC)

/* DHR registers offsets */
#define DHR12R1_OFFSET             ((uint32_t)0x00000008)
#define DHR12R2_OFFSET             ((uint32_t)0x00000014)
#define DHR12RD_OFFSET             ((uint32_t)0x00000020)

/* DOR register offset */
#define DOR_OFFSET                 ((uint32_t)0x0000002C)
  
#ifdef __cplusplus
namespace stm32 {
#endif
  
/**
  * @brief  Initializes the DAC peripheral according to the specified parameters
  *         in the DAC_InitStruct.
  * @param  DAC_Channel: the selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  DAC_InitStruct: pointer to a DAC_InitTypeDef structure that contains
  *         the configuration information for the  specified DAC channel.
  * @retval None
  */
static inline void dac_init(uint32_t dac_channel, uint32_t trigger, uint32_t wave_gen, 
		uint32_t lsfr_unamsk_triangle, uint32_t output_buffer )
{
  /* Get the DAC CR value */
  uint32_t tmpreg1 = DAC->CR;
  /* Clear BOFFx, TENx, TSELx, WAVEx and MAMPx bits */
  tmpreg1 &= ~(CR_CLEAR_MASK << dac_channel);
  /* Configure for the selected DAC channel: buffer output, trigger, 
     wave generation, mask/amplitude for wave generation */
  /* Set TSELx and TENx bits according to DAC_Trigger value */
  /* Set WAVEx bits according to DAC_WaveGeneration value */
  /* Set MAMPx bits according to DAC_LFSRUnmask_TriangleAmplitude value */ 
  /* Set BOFFx bit according to DAC_OutputBuffer value */   
  uint32_t tmpreg2 = trigger | wave_gen | lsfr_unamsk_triangle | output_buffer;
  /* Calculate CR register value depending on DAC_Channel */
  tmpreg1 |= tmpreg2 << dac_channel;
  /* Write to DAC CR */
  DAC->CR = tmpreg1;
}
  
/**
  * @brief  Enables or disables the specified DAC channel.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  NewState: new state of the DAC channel. 
  *          This parameter can be: ENABLE or DISABLE.
  * @note   When the DAC channel is enabled the trigger source can no more be modified.
  * @retval None
  */
static inline void dac_cmd(uint32_t dac_channel, bool enabled)
{

  if ( enabled )
  {
    /* Enable the selected DAC channel */
    DAC->CR |= (DAC_CR_EN1 << dac_channel);
  }
  else
  {
    /* Disable the selected DAC channel */
    DAC->CR &= (~(DAC_CR_EN1 << dac_channel));
  }
}
  
/**
  * @brief  Enables or disables the selected DAC channel software trigger.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  NewState: new state of the selected DAC channel software trigger.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void dac_software_trigger_cmd(uint32_t dac_channel, bool enable)
{
  if ( enable )
  {
    /* Enable software trigger for the selected DAC channel */
    DAC->SWTRIGR |= (uint32_t)DAC_SWTRIGR_SWTRIG1 << (dac_channel >> 4);
  }
  else
  {
    /* Disable software trigger for the selected DAC channel */
    DAC->SWTRIGR &= ~((uint32_t)DAC_SWTRIGR_SWTRIG1 << (dac_channel >> 4));
  }
}
  
/**
  * @brief  Enables or disables simultaneously the two DAC channels software triggers.
  * @param  NewState: new state of the DAC channels software triggers.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void dac_dual_software_trigger_cmd( bool enable )
{
  if ( enable )
  {
    /* Enable software trigger for both DAC channels */
    DAC->SWTRIGR |= DUAL_SWTRIG_SET;
  }
  else
  {
    /* Disable software trigger for both DAC channels */
    DAC->SWTRIGR &= DUAL_SWTRIG_RESET;
  }
}
  
/**
  * @brief  Enables or disables the selected DAC channel wave generation.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  DAC_Wave: specifies the wave type to enable or disable.
  *          This parameter can be one of the following values:
  *            @arg DAC_Wave_Noise: noise wave generation
  *            @arg DAC_Wave_Triangle: triangle wave generation
  * @param  NewState: new state of the selected DAC channel wave generation.
  *          This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
static inline void dac_wave_generation_cmd(uint32_t dac_channel, uint32_t dac_wave, bool enable )
{
  if ( enable )
  {
    /* Enable the selected wave generation for the selected DAC channel */
    DAC->CR |= dac_wave << dac_channel;
  }
  else
  {
    /* Disable the selected wave generation for the selected DAC channel */
    DAC->CR &= ~(dac_wave << dac_channel);
  }
}
  
/**
  * @brief  Set the specified data holding register value for DAC channel1.
  * @param  DAC_Align: Specifies the data alignment for DAC channel1.
  *          This parameter can be one of the following values:
  *            @arg DAC_Align_8b_R: 8bit right data alignment selected
  *            @arg DAC_Align_12b_L: 12bit left data alignment selected
  *            @arg DAC_Align_12b_R: 12bit right data alignment selected
  * @param  Data: Data to be loaded in the selected data holding register.
  * @retval None
  */
static inline void dac_set_channel1_data(uint32_t dac_align, uint16_t data)
{  
  uint32_t tmp = (uint32_t)DAC_BASE; 
  tmp += DHR12R1_OFFSET + dac_align;
  /* Set the DAC channel1 selected data holding register */
  *(__IO uint32_t *) tmp = data;
}
  
/**
  * @brief  Set the specified data holding register value for DAC channel2.
  * @param  DAC_Align: Specifies the data alignment for DAC channel2.
  *          This parameter can be one of the following values:
  *            @arg DAC_Align_8b_R: 8bit right data alignment selected
  *            @arg DAC_Align_12b_L: 12bit left data alignment selected
  *            @arg DAC_Align_12b_R: 12bit right data alignment selected
  * @param  Data: Data to be loaded in the selected data holding register.
  * @retval None
  */
static inline void dac_set_channel2_data(uint32_t dac_align, uint16_t data)
{
  uint32_t tmp = (uint32_t)DAC_BASE;
  tmp += DHR12R2_OFFSET + dac_align;

  /* Set the DAC channel2 selected data holding register */
  *(__IO uint32_t *)tmp = data;
}
 
/** Get adres of data dac register according to align */
static inline volatile void* dac_get_channel1_dreg( uint32_t dac_align ) 
{ 
  uint32_t tmp = (uint32_t)DAC_BASE; 
  tmp += DHR12R1_OFFSET + dac_align;
  return (volatile void*)(tmp);
}
 
/** Get adres of data dac register according to align */
static inline volatile void* dac_get_channel2_dreg( uint32_t dac_align )
{ 
  uint32_t tmp = (uint32_t)DAC_BASE; 
  tmp += DHR12R2_OFFSET + dac_align;
  return (volatile void*)(tmp);
}
 
/**
  * @brief  Set the specified data holding register value for dual channel DAC.
  * @param  DAC_Align: Specifies the data alignment for dual channel DAC.
  *          This parameter can be one of the following values:
  *            @arg DAC_Align_8b_R: 8bit right data alignment selected
  *            @arg DAC_Align_12b_L: 12bit left data alignment selected
  *            @arg DAC_Align_12b_R: 12bit right data alignment selected
  * @param  Data2: Data for DAC Channel2 to be loaded in the selected data holding register.
  * @param  Data1: Data for DAC Channel1 to be loaded in the selected data  holding register.
  * @note   In dual mode, a unique register access is required to write in both
  *          DAC channels at the same time.
  * @retval None
  */
static inline void dac_set_dual_channel_data(uint32_t dac_align, uint16_t data2, uint16_t data1)
{
  /* Calculate and set dual DAC data holding register value */
  uint32_t data;
  if (dac_align == DAC_Align_8b_R)
  {
    data = ((uint32_t)data2 << 8) | data1; 
  }
  else
  {
    data = ((uint32_t)data2 << 16) | data1;
  }
  
  uint32_t tmp = (uint32_t)DAC_BASE;
  tmp += DHR12RD_OFFSET + dac_align;

  /* Set the dual DAC selected data holding register */
  *(__IO uint32_t *)tmp = data;
}
  
/**
  * @brief  Returns the last data output value of the selected DAC channel.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @retval The selected DAC channel data output value.
  */
static inline uint16_t dac_get_data_output_value(uint32_t dac_channel)
{
  
  uint32_t tmp = (uint32_t) DAC_BASE ;
  tmp += DOR_OFFSET + ((uint32_t)dac_channel >> 2);
  
  /* Returns the DAC channel data output register value */
  return (uint16_t) (*(__IO uint32_t*) tmp);
}
  
/**
  * @brief  Enables or disables the specified DAC channel DMA request.
  * @note   When enabled DMA1 is generated when an external trigger (EXTI Line9,
  *         TIM2, TIM4, TIM5, TIM6, TIM7 or TIM8  but not a software trigger) occurs.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  NewState: new state of the selected DAC channel DMA request.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   The DAC channel1 is mapped on DMA1 Stream 5 channel7 which must be
  *          already configured.
  * @note   The DAC channel2 is mapped on DMA1 Stream 6 channel7 which must be
  *          already configured.    
  * @retval None
  */
static inline void dac_dma_cmd(uint32_t dac_channel, bool enable )
{
  if ( enable )
  {
    /* Enable the selected DAC channel DMA request */
    DAC->CR |= (DAC_CR_DMAEN1 << dac_channel);
  }
  else
  {
    /* Disable the selected DAC channel DMA request */
    DAC->CR &= (~(DAC_CR_DMAEN1 << dac_channel));
  }
}
 
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL) \
	|| defined(STM32MCU_MAJOR_TYPE_F2) || defined(STM32MCU_MAJOR_TYPE_F4)
/**
  * @brief  Enables or disables the specified DAC interrupts.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  DAC_IT: specifies the DAC interrupt sources to be enabled or disabled. 
  *          This parameter can be the following values:
  *            @arg DAC_IT_DMAUDR: DMA underrun interrupt mask
  * @note   The DMA underrun occurs when a second external trigger arrives before the 
  *         acknowledgement for the first external trigger is received (first request).
  * @param  NewState: new state of the specified DAC interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */ 
static inline void dac_it_config(uint32_t dac_channel, uint32_t dac_it, bool enable )  
{
  if ( enable )
  {
    /* Enable the selected DAC interrupts */
    DAC->CR |=  (dac_it << dac_channel);
  }
  else
  {
    /* Disable the selected DAC interrupts */
    DAC->CR &= (~(uint32_t)(dac_it << dac_channel));
  }
}
 
/**
  * @brief  Checks whether the specified DAC flag is set or not.
  * @param  DAC_Channel: thee selected DAC channel. 
  *   This parameter can be one of the following values:
  *     @arg DAC_Channel_1: DAC Channel1 selected
  *     @arg DAC_Channel_2: DAC Channel2 selected
  * @param  DAC_FLAG: specifies the flag to check. 
  *   This parameter can be only of the following value:
  *     @arg DAC_FLAG_DMAUDR: DMA underrun flag                                                 
  * @retval The new state of DAC_FLAG (SET or RESET).
  */
static inline bool dac_get_flag_status(uint32_t dac_channel, uint32_t dac_flag)
{
  /* Check the status of the specified DAC flag */
  return ((DAC->SR & (dac_flag << dac_channel)) != 0 )?(true):(false);
}
  
/**
  * @brief  Clears the DAC channel's pending flags.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  DAC_FLAG: specifies the flag to clear. 
  *          This parameter can be of the following value:
  *            @arg DAC_FLAG_DMAUDR: DMA underrun flag 
  * @note   The DMA underrun occurs when a second external trigger arrives before the 
  *         acknowledgement for the first external trigger is received (first request).                           
  * @retval None
  */
static inline void dac_clear_flag(uint32_t dac_channel, uint32_t dac_flag)
{
  /* Clear the selected DAC flags */
  DAC->SR = (dac_flag << dac_channel);
}
  
/**
  * @brief  Checks whether the specified DAC interrupt has occurred or not.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  DAC_IT: specifies the DAC interrupt source to check. 
  *          This parameter can be the following values:
  *            @arg DAC_IT_DMAUDR: DMA underrun interrupt mask
  * @note   The DMA underrun occurs when a second external trigger arrives before the 
  *         acknowledgement for the first external trigger is received (first request).
  * @retval The new state of DAC_IT (SET or RESET).
  */
static inline bool dac_get_it_status(uint32_t dac_channel, uint32_t dac_it)
{
  uint32_t enablestatus = (DAC->CR & (dac_it << dac_channel)) ;
  
  /* Check the status of the specified DAC interrupt */
  return (((DAC->SR & (dac_it << dac_channel)) != 0) && enablestatus)?(true):(false);
}
  
/**
  * @brief  Clears the DAC channel's interrupt pending bits.
  * @param  DAC_Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_Channel_1: DAC Channel1 selected
  *            @arg DAC_Channel_2: DAC Channel2 selected
  * @param  DAC_IT: specifies the DAC interrupt pending bit to clear.
  *          This parameter can be the following values:
  *            @arg DAC_IT_DMAUDR: DMA underrun interrupt mask                         
  * @note   The DMA underrun occurs when a second external trigger arrives before the 
  *         acknowledgement for the first external trigger is received (first request).                           
  * @retval None
  */
static inline void dac_clear_it_pending_bit(uint32_t dac_channel, uint32_t dac_it)
{
  /* Clear the selected DAC interrupt pending bits */
  DAC->SR = (dac_it << dac_channel);
}
  
#endif /* CPU type  */
  

  
#ifdef __cplusplus
}
#endif
  
//Undefine local defs
#undef CR_CLEAR_MASK              
#undef DUAL_SWTRIG_SET
#undef DUAL_SWTRIG_RESET
#undef DHR12R1_OFFSET
#undef DHR12R2_OFFSET
#undef DHR12RD_OFFSET
#undef DOR_OFFSET
  
#endif   /* ----- #ifndef stm32dac_INC  ----- */
