/**
  ******************************************************************************
  * @file    stm32f2xx_tim.h
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    05-March-2012 
  * @brief   This file contains all the functions prototypes for the TIM firmware 
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
#ifndef __STM32F2xx_TIM_H
#define __STM32F2xx_TIM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"

/** @addtogroup STM32F2xx_StdPeriph_Driver
  * @{
  */


/* Exported constants --------------------------------------------------------*/

/** @defgroup TIM_Exported_constants 
  * @{
  */


/** @defgroup TIM_Output_Compare_and_PWM_modes 
  * @{
  */

#define TIM_OCMode_Timing                  ((uint16_t)0x0000)
#define TIM_OCMode_Active                  ((uint16_t)0x0010)
#define TIM_OCMode_Inactive                ((uint16_t)0x0020)
#define TIM_OCMode_Toggle                  ((uint16_t)0x0030)
#define TIM_OCMode_PWM1                    ((uint16_t)0x0060)
#define TIM_OCMode_PWM2                    ((uint16_t)0x0070)

/**
  * @}
  */

/** @defgroup TIM_One_Pulse_Mode 
  * @{
  */

#define TIM_OPMode_Single                  ((uint16_t)0x0008)
#define TIM_OPMode_Repetitive              ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Channel 
  * @{
  */

#define TIM_Channel_1                      ((uint16_t)0x0000)
#define TIM_Channel_2                      ((uint16_t)0x0004)
#define TIM_Channel_3                      ((uint16_t)0x0008)
#define TIM_Channel_4                      ((uint16_t)0x000C)
                                 


/**
  * @}
  */ 

/** @defgroup TIM_Clock_Division_CKD 
  * @{
  */

#define TIM_CKD_DIV1                       ((uint16_t)0x0000)
#define TIM_CKD_DIV2                       ((uint16_t)0x0100)
#define TIM_CKD_DIV4                       ((uint16_t)0x0200)

/**
  * @}
  */

/** @defgroup TIM_Counter_Mode 
  * @{
  */

#define TIM_CounterMode_Up                 ((uint16_t)0x0000)
#define TIM_CounterMode_Down               ((uint16_t)0x0010)
#define TIM_CounterMode_CenterAligned1     ((uint16_t)0x0020)
#define TIM_CounterMode_CenterAligned2     ((uint16_t)0x0040)
#define TIM_CounterMode_CenterAligned3     ((uint16_t)0x0060)

/**
  * @}
  */ 

/** @defgroup TIM_Output_Compare_Polarity 
  * @{
  */

#define TIM_OCPolarity_High                ((uint16_t)0x0000)
#define TIM_OCPolarity_Low                 ((uint16_t)0x0002)

/**
  * @}
  */

/** @defgroup TIM_Output_Compare_N_Polarity 
  * @{
  */
  
#define TIM_OCNPolarity_High               ((uint16_t)0x0000)
#define TIM_OCNPolarity_Low                ((uint16_t)0x0008)

/**
  * @}
  */

/** @defgroup TIM_Output_Compare_State 
  * @{
  */

#define TIM_OutputState_Disable            ((uint16_t)0x0000)
#define TIM_OutputState_Enable             ((uint16_t)0x0001)

/**
  * @}
  */ 

/** @defgroup TIM_Output_Compare_N_State
  * @{
  */

#define TIM_OutputNState_Disable           ((uint16_t)0x0000)
#define TIM_OutputNState_Enable            ((uint16_t)0x0004)

/**
  * @}
  */ 

/** @defgroup TIM_Capture_Compare_State
  * @{
  */

#define TIM_CCx_Enable                      ((uint16_t)0x0001)
#define TIM_CCx_Disable                     ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Capture_Compare_N_State
  * @{
  */

#define TIM_CCxN_Enable                     ((uint16_t)0x0004)
#define TIM_CCxN_Disable                    ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Break_Input_enable_disable 
  * @{
  */

#define TIM_Break_Enable                   ((uint16_t)0x1000)
#define TIM_Break_Disable                  ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Break_Polarity 
  * @{
  */

#define TIM_BreakPolarity_Low              ((uint16_t)0x0000)
#define TIM_BreakPolarity_High             ((uint16_t)0x2000)

/**
  * @}
  */ 

/** @defgroup TIM_AOE_Bit_Set_Reset 
  * @{
  */

#define TIM_AutomaticOutput_Enable         ((uint16_t)0x4000)
#define TIM_AutomaticOutput_Disable        ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Lock_level
  * @{
  */

#define TIM_LOCKLevel_OFF                  ((uint16_t)0x0000)
#define TIM_LOCKLevel_1                    ((uint16_t)0x0100)
#define TIM_LOCKLevel_2                    ((uint16_t)0x0200)
#define TIM_LOCKLevel_3                    ((uint16_t)0x0300)

/**
  * @}
  */ 

/** @defgroup TIM_OSSI_Off_State_Selection_for_Idle_mode_state 
  * @{
  */

#define TIM_OSSIState_Enable               ((uint16_t)0x0400)
#define TIM_OSSIState_Disable              ((uint16_t)0x0000)

/**
  * @}
  */

/** @defgroup TIM_OSSR_Off_State_Selection_for_Run_mode_state
  * @{
  */

#define TIM_OSSRState_Enable               ((uint16_t)0x0800)
#define TIM_OSSRState_Disable              ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Output_Compare_Idle_State 
  * @{
  */

#define TIM_OCIdleState_Set                ((uint16_t)0x0100)
#define TIM_OCIdleState_Reset              ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Output_Compare_N_Idle_State 
  * @{
  */

#define TIM_OCNIdleState_Set               ((uint16_t)0x0200)
#define TIM_OCNIdleState_Reset             ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Input_Capture_Polarity 
  * @{
  */

#define  TIM_ICPolarity_Rising             ((uint16_t)0x0000)
#define  TIM_ICPolarity_Falling            ((uint16_t)0x0002)
#define  TIM_ICPolarity_BothEdge           ((uint16_t)0x000A)

/**
  * @}
  */ 

/** @defgroup TIM_Input_Capture_Selection 
  * @{
  */

#define TIM_ICSelection_DirectTI           ((uint16_t)0x0001) /*!< TIM Input 1, 2, 3 or 4 is selected to be 
                                                                   connected to IC1, IC2, IC3 or IC4, respectively */
#define TIM_ICSelection_IndirectTI         ((uint16_t)0x0002) /*!< TIM Input 1, 2, 3 or 4 is selected to be
                                                                   connected to IC2, IC1, IC4 or IC3, respectively. */
#define TIM_ICSelection_TRC                ((uint16_t)0x0003) /*!< TIM Input 1, 2, 3 or 4 is selected to be connected to TRC. */

/**
  * @}
  */ 

/** @defgroup TIM_Input_Capture_Prescaler 
  * @{
  */

#define TIM_ICPSC_DIV1                     ((uint16_t)0x0000) /*!< Capture performed each time an edge is detected on the capture input. */
#define TIM_ICPSC_DIV2                     ((uint16_t)0x0004) /*!< Capture performed once every 2 events. */
#define TIM_ICPSC_DIV4                     ((uint16_t)0x0008) /*!< Capture performed once every 4 events. */
#define TIM_ICPSC_DIV8                     ((uint16_t)0x000C) /*!< Capture performed once every 8 events. */

/**
  * @}
  */ 

/** @defgroup TIM_interrupt_sources 
  * @{
  */

#define TIM_IT_Update                      ((uint16_t)0x0001)
#define TIM_IT_CC1                         ((uint16_t)0x0002)
#define TIM_IT_CC2                         ((uint16_t)0x0004)
#define TIM_IT_CC3                         ((uint16_t)0x0008)
#define TIM_IT_CC4                         ((uint16_t)0x0010)
#define TIM_IT_COM                         ((uint16_t)0x0020)
#define TIM_IT_Trigger                     ((uint16_t)0x0040)
#define TIM_IT_Break                       ((uint16_t)0x0080)

/**
  * @}
  */ 

/** @defgroup TIM_DMA_Base_address 
  * @{
  */

#define TIM_DMABase_CR1                    ((uint16_t)0x0000)
#define TIM_DMABase_CR2                    ((uint16_t)0x0001)
#define TIM_DMABase_SMCR                   ((uint16_t)0x0002)
#define TIM_DMABase_DIER                   ((uint16_t)0x0003)
#define TIM_DMABase_SR                     ((uint16_t)0x0004)
#define TIM_DMABase_EGR                    ((uint16_t)0x0005)
#define TIM_DMABase_CCMR1                  ((uint16_t)0x0006)
#define TIM_DMABase_CCMR2                  ((uint16_t)0x0007)
#define TIM_DMABase_CCER                   ((uint16_t)0x0008)
#define TIM_DMABase_CNT                    ((uint16_t)0x0009)
#define TIM_DMABase_PSC                    ((uint16_t)0x000A)
#define TIM_DMABase_ARR                    ((uint16_t)0x000B)
#define TIM_DMABase_RCR                    ((uint16_t)0x000C)
#define TIM_DMABase_CCR1                   ((uint16_t)0x000D)
#define TIM_DMABase_CCR2                   ((uint16_t)0x000E)
#define TIM_DMABase_CCR3                   ((uint16_t)0x000F)
#define TIM_DMABase_CCR4                   ((uint16_t)0x0010)
#define TIM_DMABase_BDTR                   ((uint16_t)0x0011)
#define TIM_DMABase_DCR                    ((uint16_t)0x0012)
#define TIM_DMABase_OR                     ((uint16_t)0x0013)

/**
  * @}
  */ 

/** @defgroup TIM_DMA_Burst_Length 
  * @{
  */

#define TIM_DMABurstLength_1Transfer           ((uint16_t)0x0000)
#define TIM_DMABurstLength_2Transfers          ((uint16_t)0x0100)
#define TIM_DMABurstLength_3Transfers          ((uint16_t)0x0200)
#define TIM_DMABurstLength_4Transfers          ((uint16_t)0x0300)
#define TIM_DMABurstLength_5Transfers          ((uint16_t)0x0400)
#define TIM_DMABurstLength_6Transfers          ((uint16_t)0x0500)
#define TIM_DMABurstLength_7Transfers          ((uint16_t)0x0600)
#define TIM_DMABurstLength_8Transfers          ((uint16_t)0x0700)
#define TIM_DMABurstLength_9Transfers          ((uint16_t)0x0800)
#define TIM_DMABurstLength_10Transfers         ((uint16_t)0x0900)
#define TIM_DMABurstLength_11Transfers         ((uint16_t)0x0A00)
#define TIM_DMABurstLength_12Transfers         ((uint16_t)0x0B00)
#define TIM_DMABurstLength_13Transfers         ((uint16_t)0x0C00)
#define TIM_DMABurstLength_14Transfers         ((uint16_t)0x0D00)
#define TIM_DMABurstLength_15Transfers         ((uint16_t)0x0E00)
#define TIM_DMABurstLength_16Transfers         ((uint16_t)0x0F00)
#define TIM_DMABurstLength_17Transfers         ((uint16_t)0x1000)
#define TIM_DMABurstLength_18Transfers         ((uint16_t)0x1100)

/**
  * @}
  */ 

/** @defgroup TIM_DMA_sources 
  * @{
  */

#define TIM_DMA_Update                     ((uint16_t)0x0100)
#define TIM_DMA_CC1                        ((uint16_t)0x0200)
#define TIM_DMA_CC2                        ((uint16_t)0x0400)
#define TIM_DMA_CC3                        ((uint16_t)0x0800)
#define TIM_DMA_CC4                        ((uint16_t)0x1000)
#define TIM_DMA_COM                        ((uint16_t)0x2000)
#define TIM_DMA_Trigger                    ((uint16_t)0x4000)


/**
  * @}
  */ 

/** @defgroup TIM_External_Trigger_Prescaler 
  * @{
  */

#define TIM_ExtTRGPSC_OFF                  ((uint16_t)0x0000)
#define TIM_ExtTRGPSC_DIV2                 ((uint16_t)0x1000)
#define TIM_ExtTRGPSC_DIV4                 ((uint16_t)0x2000)
#define TIM_ExtTRGPSC_DIV8                 ((uint16_t)0x3000)

/**
  * @}
  */ 

/** @defgroup TIM_Internal_Trigger_Selection 
  * @{
  */

#define TIM_TS_ITR0                        ((uint16_t)0x0000)
#define TIM_TS_ITR1                        ((uint16_t)0x0010)
#define TIM_TS_ITR2                        ((uint16_t)0x0020)
#define TIM_TS_ITR3                        ((uint16_t)0x0030)
#define TIM_TS_TI1F_ED                     ((uint16_t)0x0040)
#define TIM_TS_TI1FP1                      ((uint16_t)0x0050)
#define TIM_TS_TI2FP2                      ((uint16_t)0x0060)
#define TIM_TS_ETRF                        ((uint16_t)0x0070)

/**
  * @}
  */ 

/** @defgroup TIM_TIx_External_Clock_Source 
  * @{
  */

#define TIM_TIxExternalCLK1Source_TI1      ((uint16_t)0x0050)
#define TIM_TIxExternalCLK1Source_TI2      ((uint16_t)0x0060)
#define TIM_TIxExternalCLK1Source_TI1ED    ((uint16_t)0x0040)

/**
  * @}
  */ 

/** @defgroup TIM_External_Trigger_Polarity 
  * @{
  */ 
#define TIM_ExtTRGPolarity_Inverted        ((uint16_t)0x8000)
#define TIM_ExtTRGPolarity_NonInverted     ((uint16_t)0x0000)

/**
  * @}
  */

/** @defgroup TIM_Prescaler_Reload_Mode 
  * @{
  */

#define TIM_PSCReloadMode_Update           ((uint16_t)0x0000)
#define TIM_PSCReloadMode_Immediate        ((uint16_t)0x0001)

/**
  * @}
  */ 

/** @defgroup TIM_Forced_Action 
  * @{
  */

#define TIM_ForcedAction_Active            ((uint16_t)0x0050)
#define TIM_ForcedAction_InActive          ((uint16_t)0x0040)

/**
  * @}
  */ 

/** @defgroup TIM_Encoder_Mode 
  * @{
  */

#define TIM_EncoderMode_TI1                ((uint16_t)0x0001)
#define TIM_EncoderMode_TI2                ((uint16_t)0x0002)
#define TIM_EncoderMode_TI12               ((uint16_t)0x0003)

/**
  * @}
  */ 


/** @defgroup TIM_Event_Source 
  * @{
  */

#define TIM_EventSource_Update             ((uint16_t)0x0001)
#define TIM_EventSource_CC1                ((uint16_t)0x0002)
#define TIM_EventSource_CC2                ((uint16_t)0x0004)
#define TIM_EventSource_CC3                ((uint16_t)0x0008)
#define TIM_EventSource_CC4                ((uint16_t)0x0010)
#define TIM_EventSource_COM                ((uint16_t)0x0020)
#define TIM_EventSource_Trigger            ((uint16_t)0x0040)
#define TIM_EventSource_Break              ((uint16_t)0x0080)

  
/**
  * @}
  */ 

/** @defgroup TIM_Update_Source 
  * @{
  */

#define TIM_UpdateSource_Global            ((uint16_t)0x0000) /*!< Source of update is the counter overflow/underflow
                                                                   or the setting of UG bit, or an update generation
                                                                   through the slave mode controller. */
#define TIM_UpdateSource_Regular           ((uint16_t)0x0001) /*!< Source of update is counter overflow/underflow. */

/**
  * @}
  */ 

/** @defgroup TIM_Output_Compare_Preload_State 
  * @{
  */

#define TIM_OCPreload_Enable               ((uint16_t)0x0008)
#define TIM_OCPreload_Disable              ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Output_Compare_Fast_State 
  * @{
  */

#define TIM_OCFast_Enable                  ((uint16_t)0x0004)
#define TIM_OCFast_Disable                 ((uint16_t)0x0000)

                                     
/**
  * @}
  */ 

/** @defgroup TIM_Output_Compare_Clear_State 
  * @{
  */

#define TIM_OCClear_Enable                 ((uint16_t)0x0080)
#define TIM_OCClear_Disable                ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup TIM_Trigger_Output_Source 
  * @{
  */

#define TIM_TRGOSource_Reset               ((uint16_t)0x0000)
#define TIM_TRGOSource_Enable              ((uint16_t)0x0010)
#define TIM_TRGOSource_Update              ((uint16_t)0x0020)
#define TIM_TRGOSource_OC1                 ((uint16_t)0x0030)
#define TIM_TRGOSource_OC1Ref              ((uint16_t)0x0040)
#define TIM_TRGOSource_OC2Ref              ((uint16_t)0x0050)
#define TIM_TRGOSource_OC3Ref              ((uint16_t)0x0060)
#define TIM_TRGOSource_OC4Ref              ((uint16_t)0x0070)

/**
  * @}
  */ 

/** @defgroup TIM_Slave_Mode 
  * @{
  */

#define TIM_SlaveMode_Reset                ((uint16_t)0x0004)
#define TIM_SlaveMode_Gated                ((uint16_t)0x0005)
#define TIM_SlaveMode_Trigger              ((uint16_t)0x0006)
#define TIM_SlaveMode_External1            ((uint16_t)0x0007)

/**
  * @}
  */ 

/** @defgroup TIM_Master_Slave_Mode 
  * @{
  */

#define TIM_MasterSlaveMode_Enable         ((uint16_t)0x0080)
#define TIM_MasterSlaveMode_Disable        ((uint16_t)0x0000)

/**
  * @}
  */ 
/** @defgroup TIM_Remap 
  * @{
  */

#define TIM2_TIM8_TRGO                     ((uint16_t)0x0000)
#define TIM2_ETH_PTP                       ((uint16_t)0x0400)
#define TIM2_USBFS_SOF                     ((uint16_t)0x0800)
#define TIM2_USBHS_SOF                     ((uint16_t)0x0C00)

#define TIM5_GPIO                          ((uint16_t)0x0000)
#define TIM5_LSI                           ((uint16_t)0x0040)
#define TIM5_LSE                           ((uint16_t)0x0080)
#define TIM5_RTC                           ((uint16_t)0x00C0)

#define TIM11_GPIO                         ((uint16_t)0x0000)
#define TIM11_HSE                          ((uint16_t)0x0002)


/**
  * @}
  */ 
/** @defgroup TIM_Flags 
  * @{
  */

#define TIM_FLAG_Update                    ((uint16_t)0x0001)
#define TIM_FLAG_CC1                       ((uint16_t)0x0002)
#define TIM_FLAG_CC2                       ((uint16_t)0x0004)
#define TIM_FLAG_CC3                       ((uint16_t)0x0008)
#define TIM_FLAG_CC4                       ((uint16_t)0x0010)
#define TIM_FLAG_COM                       ((uint16_t)0x0020)
#define TIM_FLAG_Trigger                   ((uint16_t)0x0040)
#define TIM_FLAG_Break                     ((uint16_t)0x0080)
#define TIM_FLAG_CC1OF                     ((uint16_t)0x0200)
#define TIM_FLAG_CC2OF                     ((uint16_t)0x0400)
#define TIM_FLAG_CC3OF                     ((uint16_t)0x0800)
#define TIM_FLAG_CC4OF                     ((uint16_t)0x1000)




/** @defgroup TIM_Legacy 
  * @{
  */

#define TIM_DMABurstLength_1Byte           TIM_DMABurstLength_1Transfer
#define TIM_DMABurstLength_2Bytes          TIM_DMABurstLength_2Transfers
#define TIM_DMABurstLength_3Bytes          TIM_DMABurstLength_3Transfers
#define TIM_DMABurstLength_4Bytes          TIM_DMABurstLength_4Transfers
#define TIM_DMABurstLength_5Bytes          TIM_DMABurstLength_5Transfers
#define TIM_DMABurstLength_6Bytes          TIM_DMABurstLength_6Transfers
#define TIM_DMABurstLength_7Bytes          TIM_DMABurstLength_7Transfers
#define TIM_DMABurstLength_8Bytes          TIM_DMABurstLength_8Transfers
#define TIM_DMABurstLength_9Bytes          TIM_DMABurstLength_9Transfers
#define TIM_DMABurstLength_10Bytes         TIM_DMABurstLength_10Transfers
#define TIM_DMABurstLength_11Bytes         TIM_DMABurstLength_11Transfers
#define TIM_DMABurstLength_12Bytes         TIM_DMABurstLength_12Transfers
#define TIM_DMABurstLength_13Bytes         TIM_DMABurstLength_13Transfers
#define TIM_DMABurstLength_14Bytes         TIM_DMABurstLength_14Transfers
#define TIM_DMABurstLength_15Bytes         TIM_DMABurstLength_15Transfers
#define TIM_DMABurstLength_16Bytes         TIM_DMABurstLength_16Transfers
#define TIM_DMABurstLength_17Bytes         TIM_DMABurstLength_17Transfers
#define TIM_DMABurstLength_18Bytes         TIM_DMABurstLength_18Transfers
/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#define _TIM_IS_ON_LIST_1( TIMx ) \
  (TIMx == TIM1) || (TIMx == TIM8)|| \
     (TIMx == TIM2) || (TIMx == TIM3)|| \
     (TIMx == TIM4) || (TIMx == TIM5)

#define _TIM_IS_ON_LIST_2( TIMx ) \
  (TIMx != TIM6) && (TIMx != TIM7)


#define _TIM_IS_ON_LIST_3( TIMx ) \
  (TIMx == TIM1) || (TIMx == TIM8)

#define _TIM_IS_ON_LIST_4( TIMx ) 1

typedef unsigned short stm32tim_oc_t;

#endif /*__STM32F2xx_TIM_H */

