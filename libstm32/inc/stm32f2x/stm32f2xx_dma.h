/**
  ******************************************************************************
  * @file    stm32f2xx_dma.h
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    05-March-2012 
  * @brief   This file contains all the functions prototypes for the DMA firmware 
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
#ifndef __STM32F2xx_DMA_H
#define __STM32F2xx_DMA_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"




/** @defgroup DMA_channel 
  * @{
  */ 
#define DMA_Channel_0                     ((uint32_t)0x00000000)
#define DMA_Channel_1                     ((uint32_t)0x02000000)
#define DMA_Channel_2                     ((uint32_t)0x04000000)
#define DMA_Channel_3                     ((uint32_t)0x06000000)
#define DMA_Channel_4                     ((uint32_t)0x08000000)
#define DMA_Channel_5                     ((uint32_t)0x0A000000)
#define DMA_Channel_6                     ((uint32_t)0x0C000000)
#define DMA_Channel_7                     ((uint32_t)0x0E000000)


/**
  * @}
  */ 


/** @defgroup DMA_data_transfer_direction 
  * @{
  */ 
#define DMA_DIR_PeripheralToMemory        ((uint32_t)0x00000000)
#define DMA_DIR_MemoryToPeripheral        ((uint32_t)0x00000040) 
#define DMA_DIR_MemoryToMemory            ((uint32_t)0x00000080)




/** @defgroup DMA_peripheral_incremented_mode 
  * @{
  */ 
#define DMA_PeripheralInc_Enable          ((uint32_t)0x00000200)
#define DMA_PeripheralInc_Disable         ((uint32_t)0x00000000)




/** @defgroup DMA_memory_incremented_mode 
  * @{
  */ 
#define DMA_MemoryInc_Enable              ((uint32_t)0x00000400)
#define DMA_MemoryInc_Disable             ((uint32_t)0x00000000)




/** @defgroup DMA_peripheral_data_size 
  * @{
  */ 
#define DMA_PeripheralDataSize_Byte       ((uint32_t)0x00000000) 
#define DMA_PeripheralDataSize_HalfWord   ((uint32_t)0x00000800) 
#define DMA_PeripheralDataSize_Word       ((uint32_t)0x00001000)




/** @defgroup DMA_memory_data_size 
  * @{
  */ 
#define DMA_MemoryDataSize_Byte           ((uint32_t)0x00000000) 
#define DMA_MemoryDataSize_HalfWord       ((uint32_t)0x00002000) 
#define DMA_MemoryDataSize_Word           ((uint32_t)0x00004000)




/** @defgroup DMA_circular_normal_mode 
  * @{
  */ 
#define DMA_Mode_Normal                   ((uint32_t)0x00000000) 
#define DMA_Mode_Circular                 ((uint32_t)0x00000100)


/** @defgroup DMA_priority_level 
  * @{
  */ 
#define DMA_Priority_Low                  ((uint32_t)0x00000000)
#define DMA_Priority_Medium               ((uint32_t)0x00010000) 
#define DMA_Priority_High                 ((uint32_t)0x00020000)
#define DMA_Priority_VeryHigh             ((uint32_t)0x00030000)





/** @defgroup DMA_fifo_direct_mode 
  * @{
  */ 
#define DMA_FIFOMode_Disable              ((uint32_t)0x00000000) 
#define DMA_FIFOMode_Enable               ((uint32_t)0x00000004)




/** @defgroup DMA_fifo_threshold_level 
  * @{
  */ 
#define DMA_FIFOThreshold_1QuarterFull    ((uint32_t)0x00000000)
#define DMA_FIFOThreshold_HalfFull        ((uint32_t)0x00000001) 
#define DMA_FIFOThreshold_3QuartersFull   ((uint32_t)0x00000002)
#define DMA_FIFOThreshold_Full            ((uint32_t)0x00000003)




/** @defgroup DMA_memory_burst 
  * @{
  */ 
#define DMA_MemoryBurst_Single            ((uint32_t)0x00000000)
#define DMA_MemoryBurst_INC4              ((uint32_t)0x00800000)  
#define DMA_MemoryBurst_INC8              ((uint32_t)0x01000000)
#define DMA_MemoryBurst_INC16             ((uint32_t)0x01800000)

/**
  * @}
  */ 


/** @defgroup DMA_peripheral_burst 
  * @{
  */ 
#define DMA_PeripheralBurst_Single        ((uint32_t)0x00000000)
#define DMA_PeripheralBurst_INC4          ((uint32_t)0x00200000)  
#define DMA_PeripheralBurst_INC8          ((uint32_t)0x00400000)
#define DMA_PeripheralBurst_INC16         ((uint32_t)0x00600000)




/** @defgroup DMA_fifo_status_level 
  * @{
  */
#define DMA_FIFOStatus_Less1QuarterFull   ((uint32_t)0x00000000 << 3)
#define DMA_FIFOStatus_1QuarterFull       ((uint32_t)0x00000001 << 3)
#define DMA_FIFOStatus_HalfFull           ((uint32_t)0x00000002 << 3) 
#define DMA_FIFOStatus_3QuartersFull      ((uint32_t)0x00000003 << 3)
#define DMA_FIFOStatus_Empty              ((uint32_t)0x00000004 << 3)
#define DMA_FIFOStatus_Full               ((uint32_t)0x00000005 << 3)




/** @defgroup DMA_flags_definition 
  * @{
  */
#define DMA_FLAG_FEIF0                    ((uint32_t)0x10800001)
#define DMA_FLAG_DMEIF0                   ((uint32_t)0x10800004)
#define DMA_FLAG_TEIF0                    ((uint32_t)0x10000008)
#define DMA_FLAG_HTIF0                    ((uint32_t)0x10000010)
#define DMA_FLAG_TCIF0                    ((uint32_t)0x10000020)
#define DMA_FLAG_FEIF1                    ((uint32_t)0x10000040)
#define DMA_FLAG_DMEIF1                   ((uint32_t)0x10000100)
#define DMA_FLAG_TEIF1                    ((uint32_t)0x10000200)
#define DMA_FLAG_HTIF1                    ((uint32_t)0x10000400)
#define DMA_FLAG_TCIF1                    ((uint32_t)0x10000800)
#define DMA_FLAG_FEIF2                    ((uint32_t)0x10010000)
#define DMA_FLAG_DMEIF2                   ((uint32_t)0x10040000)
#define DMA_FLAG_TEIF2                    ((uint32_t)0x10080000)
#define DMA_FLAG_HTIF2                    ((uint32_t)0x10100000)
#define DMA_FLAG_TCIF2                    ((uint32_t)0x10200000)
#define DMA_FLAG_FEIF3                    ((uint32_t)0x10400000)
#define DMA_FLAG_DMEIF3                   ((uint32_t)0x11000000)
#define DMA_FLAG_TEIF3                    ((uint32_t)0x12000000)
#define DMA_FLAG_HTIF3                    ((uint32_t)0x14000000)
#define DMA_FLAG_TCIF3                    ((uint32_t)0x18000000)
#define DMA_FLAG_FEIF4                    ((uint32_t)0x20000001)
#define DMA_FLAG_DMEIF4                   ((uint32_t)0x20000004)
#define DMA_FLAG_TEIF4                    ((uint32_t)0x20000008)
#define DMA_FLAG_HTIF4                    ((uint32_t)0x20000010)
#define DMA_FLAG_TCIF4                    ((uint32_t)0x20000020)
#define DMA_FLAG_FEIF5                    ((uint32_t)0x20000040)
#define DMA_FLAG_DMEIF5                   ((uint32_t)0x20000100)
#define DMA_FLAG_TEIF5                    ((uint32_t)0x20000200)
#define DMA_FLAG_HTIF5                    ((uint32_t)0x20000400)
#define DMA_FLAG_TCIF5                    ((uint32_t)0x20000800)
#define DMA_FLAG_FEIF6                    ((uint32_t)0x20010000)
#define DMA_FLAG_DMEIF6                   ((uint32_t)0x20040000)
#define DMA_FLAG_TEIF6                    ((uint32_t)0x20080000)
#define DMA_FLAG_HTIF6                    ((uint32_t)0x20100000)
#define DMA_FLAG_TCIF6                    ((uint32_t)0x20200000)
#define DMA_FLAG_FEIF7                    ((uint32_t)0x20400000)
#define DMA_FLAG_DMEIF7                   ((uint32_t)0x21000000)
#define DMA_FLAG_TEIF7                    ((uint32_t)0x22000000)
#define DMA_FLAG_HTIF7                    ((uint32_t)0x24000000)
#define DMA_FLAG_TCIF7                    ((uint32_t)0x28000000)


/**
  * @}
  */ 


/** @defgroup DMA_interrupt_enable_definitions 
  * @{
  */ 
#define DMA_IT_TC                         ((uint32_t)0x00000010)
#define DMA_IT_HT                         ((uint32_t)0x00000008)
#define DMA_IT_TE                         ((uint32_t)0x00000004)
#define DMA_IT_DME                        ((uint32_t)0x00000002)
#define DMA_IT_FE                         ((uint32_t)0x00000080)




/** @defgroup DMA_interrupts_definitions 
  * @{
  */ 
#define DMA_IT_FEIF0                      ((uint32_t)0x90000001)
#define DMA_IT_DMEIF0                     ((uint32_t)0x10001004)
#define DMA_IT_TEIF0                      ((uint32_t)0x10002008)
#define DMA_IT_HTIF0                      ((uint32_t)0x10004010)
#define DMA_IT_TCIF0                      ((uint32_t)0x10008020)
#define DMA_IT_FEIF1                      ((uint32_t)0x90000040)
#define DMA_IT_DMEIF1                     ((uint32_t)0x10001100)
#define DMA_IT_TEIF1                      ((uint32_t)0x10002200)
#define DMA_IT_HTIF1                      ((uint32_t)0x10004400)
#define DMA_IT_TCIF1                      ((uint32_t)0x10008800)
#define DMA_IT_FEIF2                      ((uint32_t)0x90010000)
#define DMA_IT_DMEIF2                     ((uint32_t)0x10041000)
#define DMA_IT_TEIF2                      ((uint32_t)0x10082000)
#define DMA_IT_HTIF2                      ((uint32_t)0x10104000)
#define DMA_IT_TCIF2                      ((uint32_t)0x10208000)
#define DMA_IT_FEIF3                      ((uint32_t)0x90400000)
#define DMA_IT_DMEIF3                     ((uint32_t)0x11001000)
#define DMA_IT_TEIF3                      ((uint32_t)0x12002000)
#define DMA_IT_HTIF3                      ((uint32_t)0x14004000)
#define DMA_IT_TCIF3                      ((uint32_t)0x18008000)
#define DMA_IT_FEIF4                      ((uint32_t)0xA0000001)
#define DMA_IT_DMEIF4                     ((uint32_t)0x20001004)
#define DMA_IT_TEIF4                      ((uint32_t)0x20002008)
#define DMA_IT_HTIF4                      ((uint32_t)0x20004010)
#define DMA_IT_TCIF4                      ((uint32_t)0x20008020)
#define DMA_IT_FEIF5                      ((uint32_t)0xA0000040)
#define DMA_IT_DMEIF5                     ((uint32_t)0x20001100)
#define DMA_IT_TEIF5                      ((uint32_t)0x20002200)
#define DMA_IT_HTIF5                      ((uint32_t)0x20004400)
#define DMA_IT_TCIF5                      ((uint32_t)0x20008800)
#define DMA_IT_FEIF6                      ((uint32_t)0xA0010000)
#define DMA_IT_DMEIF6                     ((uint32_t)0x20041000)
#define DMA_IT_TEIF6                      ((uint32_t)0x20082000)
#define DMA_IT_HTIF6                      ((uint32_t)0x20104000)
#define DMA_IT_TCIF6                      ((uint32_t)0x20208000)
#define DMA_IT_FEIF7                      ((uint32_t)0xA0400000)
#define DMA_IT_DMEIF7                     ((uint32_t)0x21001000)
#define DMA_IT_TEIF7                      ((uint32_t)0x22002000)
#define DMA_IT_HTIF7                      ((uint32_t)0x24004000)
#define DMA_IT_TCIF7                      ((uint32_t)0x28008000)




/** @defgroup DMA_peripheral_increment_offset 
  * @{
  */ 
#define DMA_PINCOS_Psize                  ((uint32_t)0x00000000)
#define DMA_PINCOS_WordAligned            ((uint32_t)0x00008000)



/** @defgroup DMA_flow_controller_definitions 
  * @{
  */ 
#define DMA_FlowCtrl_Memory               ((uint32_t)0x00000000)
#define DMA_FlowCtrl_Peripheral           ((uint32_t)0x00000020)




/** @defgroup DMA_memory_targets_definitions 
  * @{
  */ 
#define DMA_Memory_0                      ((uint32_t)0x00000000)
#define DMA_Memory_1                      ((uint32_t)0x00080000)




#ifdef __cplusplus
}
#endif

#endif /*__STM32F2xx_DMA_H */

/**
  * @}
  */

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
