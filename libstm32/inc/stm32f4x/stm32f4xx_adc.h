/**
  ******************************************************************************
  * @file    stm32f4xx_adc.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    05-March-2012
  * @brief   This file contains all the functions prototypes for the ADC firmware 
  *          library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4xx_ADC_H
#define __STM32F4xx_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"




/* Exported constants --------------------------------------------------------*/

/** @defgroup ADC_Exported_Constants
  * @{
  */ 
#define IS_ADC_ALL_PERIPH(PERIPH) (((PERIPH) == ADC1) || \
                                   ((PERIPH) == ADC2) || \
                                   ((PERIPH) == ADC3))  

/** @defgroup ADC_Common_mode 
  * @{
  */ 
#define ADC_Mode_Independent                       ((uint32_t)0x00000000)       
#define ADC_DualMode_RegSimult_InjecSimult         ((uint32_t)0x00000001)
#define ADC_DualMode_RegSimult_AlterTrig           ((uint32_t)0x00000002)
#define ADC_DualMode_InjecSimult                   ((uint32_t)0x00000005)
#define ADC_DualMode_RegSimult                     ((uint32_t)0x00000006)
#define ADC_DualMode_Interl                        ((uint32_t)0x00000007)
#define ADC_DualMode_AlterTrig                     ((uint32_t)0x00000009)
#define ADC_TripleMode_RegSimult_InjecSimult       ((uint32_t)0x00000011)
#define ADC_TripleMode_RegSimult_AlterTrig         ((uint32_t)0x00000012)
#define ADC_TripleMode_InjecSimult                 ((uint32_t)0x00000015)
#define ADC_TripleMode_RegSimult                   ((uint32_t)0x00000016)
#define ADC_TripleMode_Interl                      ((uint32_t)0x00000017)
#define ADC_TripleMode_AlterTrig                   ((uint32_t)0x00000019)

/**
  * @}
  */ 


/** @defgroup ADC_Prescaler 
  * @{
  */ 
#define ADC_Prescaler_Div2                         ((uint32_t)0x00000000)
#define ADC_Prescaler_Div4                         ((uint32_t)0x00010000)
#define ADC_Prescaler_Div6                         ((uint32_t)0x00020000)
#define ADC_Prescaler_Div8                         ((uint32_t)0x00030000)

/**
  * @}
  */ 


/** @defgroup ADC_Direct_memory_access_mode_for_multi_mode 
  * @{
  */ 
#define ADC_DMAAccessMode_Disabled      ((uint32_t)0x00000000)     /* DMA mode disabled */
#define ADC_DMAAccessMode_1             ((uint32_t)0x00004000)     /* DMA mode 1 enabled (2 / 3 half-words one by one - 1 then 2 then 3)*/
#define ADC_DMAAccessMode_2             ((uint32_t)0x00008000)     /* DMA mode 2 enabled (2 / 3 half-words by pairs - 2&1 then 1&3 then 3&2)*/
#define ADC_DMAAccessMode_3             ((uint32_t)0x0000C000)     /* DMA mode 3 enabled (2 / 3 bytes by pairs - 2&1 then 1&3 then 3&2) */

                                     
/**
  * @}
  */ 


/** @defgroup ADC_delay_between_2_sampling_phases 
  * @{
  */ 
#define ADC_TwoSamplingDelay_5Cycles               ((uint32_t)0x00000000)
#define ADC_TwoSamplingDelay_6Cycles               ((uint32_t)0x00000100)
#define ADC_TwoSamplingDelay_7Cycles               ((uint32_t)0x00000200)
#define ADC_TwoSamplingDelay_8Cycles               ((uint32_t)0x00000300)
#define ADC_TwoSamplingDelay_9Cycles               ((uint32_t)0x00000400)
#define ADC_TwoSamplingDelay_10Cycles              ((uint32_t)0x00000500)
#define ADC_TwoSamplingDelay_11Cycles              ((uint32_t)0x00000600)
#define ADC_TwoSamplingDelay_12Cycles              ((uint32_t)0x00000700)
#define ADC_TwoSamplingDelay_13Cycles              ((uint32_t)0x00000800)
#define ADC_TwoSamplingDelay_14Cycles              ((uint32_t)0x00000900)
#define ADC_TwoSamplingDelay_15Cycles              ((uint32_t)0x00000A00)
#define ADC_TwoSamplingDelay_16Cycles              ((uint32_t)0x00000B00)
#define ADC_TwoSamplingDelay_17Cycles              ((uint32_t)0x00000C00)
#define ADC_TwoSamplingDelay_18Cycles              ((uint32_t)0x00000D00)
#define ADC_TwoSamplingDelay_19Cycles              ((uint32_t)0x00000E00)
#define ADC_TwoSamplingDelay_20Cycles              ((uint32_t)0x00000F00)

                                     
/**
  * @}
  */ 


/** @defgroup ADC_resolution 
  * @{
  */ 
#define ADC_Resolution_12b                         ((uint32_t)0x00000000)
#define ADC_Resolution_10b                         ((uint32_t)0x01000000)
#define ADC_Resolution_8b                          ((uint32_t)0x02000000)
#define ADC_Resolution_6b                          ((uint32_t)0x03000000)

                                      
/**
  * @}
  */ 


/** @defgroup ADC_external_trigger_edge_for_regular_channels_conversion 
  * @{
  */ 
#define ADC_ExternalTrigConvEdge_None          ((uint32_t)0x00000000)
#define ADC_ExternalTrigConvEdge_Rising        ((uint32_t)0x10000000)
#define ADC_ExternalTrigConvEdge_Falling       ((uint32_t)0x20000000)
#define ADC_ExternalTrigConvEdge_RisingFalling ((uint32_t)0x30000000)

/**
  * @}
  */ 


/** @defgroup ADC_extrenal_trigger_sources_for_regular_channels_conversion 
  * @{
  */ 
#define ADC_ExternalTrigConv_T1_CC1                ((uint32_t)0x00000000)
#define ADC_ExternalTrigConv_T1_CC2                ((uint32_t)0x01000000)
#define ADC_ExternalTrigConv_T1_CC3                ((uint32_t)0x02000000)
#define ADC_ExternalTrigConv_T2_CC2                ((uint32_t)0x03000000)
#define ADC_ExternalTrigConv_T2_CC3                ((uint32_t)0x04000000)
#define ADC_ExternalTrigConv_T2_CC4                ((uint32_t)0x05000000)
#define ADC_ExternalTrigConv_T2_TRGO               ((uint32_t)0x06000000)
#define ADC_ExternalTrigConv_T3_CC1                ((uint32_t)0x07000000)
#define ADC_ExternalTrigConv_T3_TRGO               ((uint32_t)0x08000000)
#define ADC_ExternalTrigConv_T4_CC4                ((uint32_t)0x09000000)
#define ADC_ExternalTrigConv_T5_CC1                ((uint32_t)0x0A000000)
#define ADC_ExternalTrigConv_T5_CC2                ((uint32_t)0x0B000000)
#define ADC_ExternalTrigConv_T5_CC3                ((uint32_t)0x0C000000)
#define ADC_ExternalTrigConv_T8_CC1                ((uint32_t)0x0D000000)
#define ADC_ExternalTrigConv_T8_TRGO               ((uint32_t)0x0E000000)
#define ADC_ExternalTrigConv_Ext_IT11              ((uint32_t)0x0F000000)

/**
  * @}
  */ 


/** @defgroup ADC_data_align 
  * @{
  */ 
#define ADC_DataAlign_Right                        ((uint32_t)0x00000000)
#define ADC_DataAlign_Left                         ((uint32_t)0x00000800)


/** @defgroup ADC_channels 
  * @{
  */ 
#define ADC_Channel_0                               ((uint8_t)0x00)
#define ADC_Channel_1                               ((uint8_t)0x01)
#define ADC_Channel_2                               ((uint8_t)0x02)
#define ADC_Channel_3                               ((uint8_t)0x03)
#define ADC_Channel_4                               ((uint8_t)0x04)
#define ADC_Channel_5                               ((uint8_t)0x05)
#define ADC_Channel_6                               ((uint8_t)0x06)
#define ADC_Channel_7                               ((uint8_t)0x07)
#define ADC_Channel_8                               ((uint8_t)0x08)
#define ADC_Channel_9                               ((uint8_t)0x09)
#define ADC_Channel_10                              ((uint8_t)0x0A)
#define ADC_Channel_11                              ((uint8_t)0x0B)
#define ADC_Channel_12                              ((uint8_t)0x0C)
#define ADC_Channel_13                              ((uint8_t)0x0D)
#define ADC_Channel_14                              ((uint8_t)0x0E)
#define ADC_Channel_15                              ((uint8_t)0x0F)
#define ADC_Channel_16                              ((uint8_t)0x10)
#define ADC_Channel_17                              ((uint8_t)0x11)
#define ADC_Channel_18                              ((uint8_t)0x12)

#define ADC_Channel_TempSensor                      ((uint8_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                         ((uint8_t)ADC_Channel_17)
#define ADC_Channel_Vbat                            ((uint8_t)ADC_Channel_18)


/**
  * @}
  */ 


/** @defgroup ADC_sampling_times 
  * @{
  */ 
#define ADC_SampleTime_3Cycles                    ((uint8_t)0x00)
#define ADC_SampleTime_15Cycles                   ((uint8_t)0x01)
#define ADC_SampleTime_28Cycles                   ((uint8_t)0x02)
#define ADC_SampleTime_56Cycles                   ((uint8_t)0x03)
#define ADC_SampleTime_84Cycles                   ((uint8_t)0x04)
#define ADC_SampleTime_112Cycles                  ((uint8_t)0x05)
#define ADC_SampleTime_144Cycles                  ((uint8_t)0x06)
#define ADC_SampleTime_480Cycles                  ((uint8_t)0x07)

/**
  * @}
  */ 


/** @defgroup ADC_external_trigger_edge_for_injected_channels_conversion 
  * @{
  */ 
#define ADC_ExternalTrigInjecConvEdge_None          ((uint32_t)0x00000000)
#define ADC_ExternalTrigInjecConvEdge_Rising        ((uint32_t)0x00100000)
#define ADC_ExternalTrigInjecConvEdge_Falling       ((uint32_t)0x00200000)
#define ADC_ExternalTrigInjecConvEdge_RisingFalling ((uint32_t)0x00300000)

                                            
/**
  * @}
  */ 


/** @defgroup ADC_extrenal_trigger_sources_for_injected_channels_conversion 
  * @{
  */ 
#define ADC_ExternalTrigInjecConv_T1_CC4            ((uint32_t)0x00000000)
#define ADC_ExternalTrigInjecConv_T1_TRGO           ((uint32_t)0x00010000)
#define ADC_ExternalTrigInjecConv_T2_CC1            ((uint32_t)0x00020000)
#define ADC_ExternalTrigInjecConv_T2_TRGO           ((uint32_t)0x00030000)
#define ADC_ExternalTrigInjecConv_T3_CC2            ((uint32_t)0x00040000)
#define ADC_ExternalTrigInjecConv_T3_CC4            ((uint32_t)0x00050000)
#define ADC_ExternalTrigInjecConv_T4_CC1            ((uint32_t)0x00060000)
#define ADC_ExternalTrigInjecConv_T4_CC2            ((uint32_t)0x00070000)
#define ADC_ExternalTrigInjecConv_T4_CC3            ((uint32_t)0x00080000)
#define ADC_ExternalTrigInjecConv_T4_TRGO           ((uint32_t)0x00090000)
#define ADC_ExternalTrigInjecConv_T5_CC4            ((uint32_t)0x000A0000)
#define ADC_ExternalTrigInjecConv_T5_TRGO           ((uint32_t)0x000B0000)
#define ADC_ExternalTrigInjecConv_T8_CC2            ((uint32_t)0x000C0000)
#define ADC_ExternalTrigInjecConv_T8_CC3            ((uint32_t)0x000D0000)
#define ADC_ExternalTrigInjecConv_T8_CC4            ((uint32_t)0x000E0000)
#define ADC_ExternalTrigInjecConv_Ext_IT15          ((uint32_t)0x000F0000)

/**
  * @}
  */ 


/** @defgroup ADC_injected_channel_selection 
  * @{
  */ 
#define ADC_InjectedChannel_1                       ((uint8_t)0x14)
#define ADC_InjectedChannel_2                       ((uint8_t)0x18)
#define ADC_InjectedChannel_3                       ((uint8_t)0x1C)
#define ADC_InjectedChannel_4                       ((uint8_t)0x20)

/**
  * @}
  */ 


/** @defgroup ADC_analog_watchdog_selection 
  * @{
  */ 
#define ADC_AnalogWatchdog_SingleRegEnable         ((uint32_t)0x00800200)
#define ADC_AnalogWatchdog_SingleInjecEnable       ((uint32_t)0x00400200)
#define ADC_AnalogWatchdog_SingleRegOrInjecEnable  ((uint32_t)0x00C00200)
#define ADC_AnalogWatchdog_AllRegEnable            ((uint32_t)0x00800000)
#define ADC_AnalogWatchdog_AllInjecEnable          ((uint32_t)0x00400000)
#define ADC_AnalogWatchdog_AllRegAllInjecEnable    ((uint32_t)0x00C00000)
#define ADC_AnalogWatchdog_None                    ((uint32_t)0x00000000)

/**
  * @}
  */ 


/** @defgroup ADC_interrupts_definition 
  * @{
  */ 
#define ADC_IT_EOC                                 ((uint16_t)0x0205)  
#define ADC_IT_AWD                                 ((uint16_t)0x0106)  
#define ADC_IT_JEOC                                ((uint16_t)0x0407)  
#define ADC_IT_OVR                                 ((uint16_t)0x201A)  

/**
  * @}
  */ 


/** @defgroup ADC_flags_definition 
  * @{
  */ 
#define ADC_FLAG_AWD                               ((uint8_t)0x01)
#define ADC_FLAG_EOC                               ((uint8_t)0x02)
#define ADC_FLAG_JEOC                              ((uint8_t)0x04)
#define ADC_FLAG_JSTRT                             ((uint8_t)0x08)
#define ADC_FLAG_STRT                              ((uint8_t)0x10)
#define ADC_FLAG_OVR                               ((uint8_t)0x20)   
  


#ifdef __cplusplus
}
#endif

#endif /*__STM32F4xx_ADC_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
