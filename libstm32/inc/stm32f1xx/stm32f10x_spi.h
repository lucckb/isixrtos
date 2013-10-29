/**
  ******************************************************************************
  * @file    stm32f10x_spi.h
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    11-March-2011
  * @brief   This file contains all the functions prototypes for the SPI firmware 
  *          library.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_SPI_H
#define __STM32F10x_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


  
#define SPI_Direction_2Lines_FullDuplex ((uint16_t)0x0000)
#define SPI_Direction_2Lines_RxOnly     ((uint16_t)0x0400)
#define SPI_Direction_1Line_Rx          ((uint16_t)0x8000)
#define SPI_Direction_1Line_Tx          ((uint16_t)0xC000)

/**
  * @}
  */

/** @defgroup SPI_mode 
  * @{
  */

#define SPI_Mode_Master                 ((uint16_t)0x0104)
#define SPI_Mode_Slave                  ((uint16_t)0x0000)


/** @defgroup SPI_data_size 
  * @{
  */

#define SPI_DataSize_16b                ((uint16_t)0x0800)
#define SPI_DataSize_8b                 ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup SPI_Clock_Polarity 
  * @{
  */

#define SPI_CPOL_Low                    ((uint16_t)0x0000)
#define SPI_CPOL_High                   ((uint16_t)0x0002)

/**
  * @}
  */

/** @defgroup SPI_Clock_Phase 
  * @{
  */

#define SPI_CPHA_1Edge                  ((uint16_t)0x0000)
#define SPI_CPHA_2Edge                  ((uint16_t)0x0001)

/**
  * @}
  */

/** @defgroup SPI_Slave_Select_management 
  * @{
  */

#define SPI_NSS_Soft                    ((uint16_t)0x0200)
#define SPI_NSS_Hard                    ((uint16_t)0x0000)

/**
  * @}
  */ 

/** @defgroup SPI_BaudRate_Prescaler 
  * @{
  */

#define SPI_BaudRatePrescaler_2         ((uint16_t)0x0000)
#define SPI_BaudRatePrescaler_4         ((uint16_t)0x0008)
#define SPI_BaudRatePrescaler_8         ((uint16_t)0x0010)
#define SPI_BaudRatePrescaler_16        ((uint16_t)0x0018)
#define SPI_BaudRatePrescaler_32        ((uint16_t)0x0020)
#define SPI_BaudRatePrescaler_64        ((uint16_t)0x0028)
#define SPI_BaudRatePrescaler_128       ((uint16_t)0x0030)
#define SPI_BaudRatePrescaler_256       ((uint16_t)0x0038)

/**
  * @}
  */ 

/** @defgroup SPI_MSB_LSB_transmission 
  * @{
  */

#define SPI_FirstBit_MSB                ((uint16_t)0x0000)
#define SPI_FirstBit_LSB                ((uint16_t)0x0080)

/**
  * @}
  */

/** @defgroup I2S_Mode 
  * @{
  */

#define I2S_Mode_SlaveTx                ((uint16_t)0x0000)
#define I2S_Mode_SlaveRx                ((uint16_t)0x0100)
#define I2S_Mode_MasterTx               ((uint16_t)0x0200)
#define I2S_Mode_MasterRx               ((uint16_t)0x0300)

/**
  * @}
  */

/** @defgroup I2S_Standard 
  * @{
  */

#define I2S_Standard_Phillips           ((uint16_t)0x0000)
#define I2S_Standard_MSB                ((uint16_t)0x0010)
#define I2S_Standard_LSB                ((uint16_t)0x0020)
#define I2S_Standard_PCMShort           ((uint16_t)0x0030)
#define I2S_Standard_PCMLong            ((uint16_t)0x00B0)

/**
  * @}
  */

/** @defgroup I2S_Data_Format 
  * @{
  */

#define I2S_DataFormat_16b              ((uint16_t)0x0000)
#define I2S_DataFormat_16bextended      ((uint16_t)0x0001)
#define I2S_DataFormat_24b              ((uint16_t)0x0003)
#define I2S_DataFormat_32b              ((uint16_t)0x0005)

/**
  * @}
  */ 

/** @defgroup I2S_MCLK_Output 
  * @{
  */

#define I2S_MCLKOutput_Enable           ((uint16_t)0x0200)
#define I2S_MCLKOutput_Disable          ((uint16_t)0x0000)

/**
  * @}
  */

/** @defgroup I2S_Audio_Frequency 
  * @{
  */

#define I2S_AudioFreq_192k               ((uint32_t)192000)
#define I2S_AudioFreq_96k                ((uint32_t)96000)
#define I2S_AudioFreq_48k                ((uint32_t)48000)
#define I2S_AudioFreq_44k                ((uint32_t)44100)
#define I2S_AudioFreq_32k                ((uint32_t)32000)
#define I2S_AudioFreq_22k                ((uint32_t)22050)
#define I2S_AudioFreq_16k                ((uint32_t)16000)
#define I2S_AudioFreq_11k                ((uint32_t)11025)
#define I2S_AudioFreq_8k                 ((uint32_t)8000)
#define I2S_AudioFreq_Default            ((uint32_t)2)


/**
  * @}
  */ 

/** @defgroup I2S_Clock_Polarity 
  * @{
  */

#define I2S_CPOL_Low                    ((uint16_t)0x0000)
#define I2S_CPOL_High                   ((uint16_t)0x0008)

/**
  * @}
  */

/** @defgroup SPI_I2S_DMA_transfer_requests 
  * @{
  */

#define SPI_I2S_DMAReq_Tx               ((uint16_t)0x0002)
#define SPI_I2S_DMAReq_Rx               ((uint16_t)0x0001)

/**
  * @}
  */

/** @defgroup SPI_NSS_internal_software_management 
  * @{
  */

#define SPI_NSSInternalSoft_Set         ((uint16_t)0x0100)
#define SPI_NSSInternalSoft_Reset       ((uint16_t)0xFEFF)

/**
  * @}
  */

/** @defgroup SPI_CRC_Transmit_Receive 
  * @{
  */

#define SPI_CRC_Tx                      ((uint8_t)0x00)
#define SPI_CRC_Rx                      ((uint8_t)0x01)

/**
  * @}
  */

/** @defgroup SPI_direction_transmit_receive 
  * @{
  */

#define SPI_Direction_Rx                ((uint16_t)0xBFFF)
#define SPI_Direction_Tx                ((uint16_t)0x4000)

/**
  * @}
  */

/** @defgroup SPI_I2S_interrupts_definition 
  * @{
  */

#define SPI_I2S_IT_TXE                  ((uint8_t)0x71)
#define SPI_I2S_IT_RXNE                 ((uint8_t)0x60)
#define SPI_I2S_IT_ERR                  ((uint8_t)0x50)

#define SPI_I2S_IT_OVR                  ((uint8_t)0x56)
#define SPI_IT_MODF                     ((uint8_t)0x55)
#define SPI_IT_CRCERR                   ((uint8_t)0x54)
#define I2S_IT_UDR                      ((uint8_t)0x53)

/**
  * @}
  */

/** @defgroup SPI_I2S_flags_definition 
  * @{
  */

#define SPI_I2S_FLAG_RXNE               ((uint16_t)0x0001)
#define SPI_I2S_FLAG_TXE                ((uint16_t)0x0002)
#define I2S_FLAG_CHSIDE                 ((uint16_t)0x0004)
#define I2S_FLAG_UDR                    ((uint16_t)0x0008)
#define SPI_FLAG_CRCERR                 ((uint16_t)0x0010)
#define SPI_FLAG_MODF                   ((uint16_t)0x0020)
#define SPI_I2S_FLAG_OVR                ((uint16_t)0x0040)
#define SPI_I2S_FLAG_BSY                ((uint16_t)0x0080)

/**
  * @}
  */

/** @defgroup SPI_CRC_polynomial
  * @{
  */



#ifdef __cplusplus
}
#endif

#endif /*__STM32F10x_SPI_H */
