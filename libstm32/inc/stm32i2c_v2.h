/*
 * =====================================================================================
 *
 *       Filename:  stm32i2c_v2.h
 *
 *    Description:  STM32 new hardware for STM32F3
 *
 *        Version:  1.0
 *        Created:  29.03.2016 13:45:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "stm32f3_i2c.h"
#include "stm32lib.h"


#define CR1_CLEAR_MASK          ((uint32_t)0x00CFE0FF)  /*<! I2C CR1 clear register Mask */
#define CR2_CLEAR_MASK          ((uint32_t)0x07FF7FFF)  /*<! I2C CR2 clear register Mask */
#define TIMING_CLEAR_MASK       ((uint32_t)0xF0FFFFFF)  /*<! I2C TIMING clear register Mask */
#define ERROR_IT_MASK           ((uint32_t)0x00003F00)  /*<! I2C Error interrupt register Mask */
#define TC_IT_MASK              ((uint32_t)0x000000C0)  /*<! I2C TC interrupt register Mask */

#ifdef __cplusplus
namespace stm32 {
#endif


/**
  * @brief  Initializes the I2Cx peripheral according to the specified
  *         parameters in the I2C_InitStruct.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_InitStruct: pointer to a I2C_InitTypeDef structure that
  *         contains the configuration information for the specified I2C peripheral.
  * @retval None
  */
static inline void i2c_init(I2C_TypeDef* I2Cx, uint32_t timing, uint32_t analog_filter, 
		uint32_t digital_filter, uint32_t mode, uint32_t own_address1, uint32_t ack,
		uint32_t acknowledged_address )
{
  uint32_t tmpreg = 0;

  /* Disable I2Cx Peripheral */
  I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_PE);

  /*---------------------------- I2Cx FILTERS Configuration ------------------*/
  /* Get the I2Cx CR1 value */
  tmpreg = I2Cx->CR1;
  /* Clear I2Cx CR1 register */
  tmpreg &= CR1_CLEAR_MASK;
  /* Configure I2Cx: analog and digital filter */
  /* Set ANFOFF bit according to I2C_AnalogFilter value */
  /* Set DFN bits according to I2C_DigitalFilter value */
  tmpreg |= (uint32_t)analog_filter |(digital_filter << 8);
  
  /* Write to I2Cx CR1 */
  I2Cx->CR1 = tmpreg;

  /*---------------------------- I2Cx TIMING Configuration -------------------*/
  /* Configure I2Cx: Timing */
  /* Set TIMINGR bits according to I2C_Timing */
  /* Write to I2Cx TIMING */
  I2Cx->TIMINGR = timing & TIMING_CLEAR_MASK;

  /* Enable I2Cx Peripheral */
  I2Cx->CR1 |= I2C_CR1_PE;

  /*---------------------------- I2Cx OAR1 Configuration ---------------------*/
  /* Clear tmpreg local variable */
  tmpreg = 0;
  /* Clear OAR1 register */
  I2Cx->OAR1 = (uint32_t)tmpreg;
  /* Clear OAR2 register */
  I2Cx->OAR2 = (uint32_t)tmpreg;
  /* Configure I2Cx: Own Address1 and acknowledged address */
  /* Set OA1MODE bit according to I2C_AcknowledgedAddress value */
  /* Set OA1 bits according to I2C_OwnAddress1 value */
  tmpreg = (uint32_t)((uint32_t)acknowledged_address | \
                      (uint32_t)own_address1);
  /* Write to I2Cx OAR1 */
  I2Cx->OAR1 = tmpreg;
  /* Enable Own Address1 acknowledgement */
  I2Cx->OAR1 |= I2C_OAR1_OA1EN;

  /*---------------------------- I2Cx MODE Configuration ---------------------*/
  /* Configure I2Cx: mode */
  /* Set SMBDEN and SMBHEN bits according to I2C_Mode value */
  tmpreg = mode;
  /* Write to I2Cx CR1 */
  I2Cx->CR1 |= tmpreg;

  /*---------------------------- I2Cx ACK Configuration ----------------------*/
  /* Get the I2Cx CR2 value */
  tmpreg = I2Cx->CR2;
  /* Clear I2Cx CR2 register */
  tmpreg &= CR2_CLEAR_MASK;
  /* Configure I2Cx: acknowledgement */
  /* Set NACK bit according to I2C_Ack value */
  tmpreg |= ack;
  /* Write to I2Cx CR2 */
  I2Cx->CR2 = tmpreg;
}

/**
  * @brief  Enables or disables the specified I2C peripheral.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx peripheral. 
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if (enabled ) {
    /* Enable the selected I2C peripheral */
    I2Cx->CR1 |= I2C_CR1_PE;
  } else {
    /* Disable the selected I2C peripheral */
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_PE);
  }
}

/**
  * @brief  Enables or disables the specified I2C software reset.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @retval None
  */
static inline void i2c_software_reset_cmd(I2C_TypeDef* I2Cx)
{
  /* Disable peripheral */
  I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_PE);
  /* Perform a dummy read to delay the disable of peripheral for minimum
     3 APB clock cycles to perform the software reset functionality */
  *(__IO uint32_t *)(uint32_t)I2Cx; 
  /* Enable peripheral */
  I2Cx->CR1 |= I2C_CR1_PE;
}

/**
  * @brief  Enables or disables the specified I2C interrupts.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_IT: specifies the I2C interrupts sources to be enabled or disabled. 
  *          This parameter can be any combination of the following values:
  *            @arg I2C_IT_ERRI: Error interrupt mask
  *            @arg I2C_IT_TCI: Transfer Complete interrupt mask
  *            @arg I2C_IT_STOPI: Stop Detection interrupt mask
  *            @arg I2C_IT_NACKI: Not Acknowledge received interrupt mask
  *            @arg I2C_IT_ADDRI: Address Match interrupt mask  
  *            @arg I2C_IT_RXI: RX interrupt mask
  *            @arg I2C_IT_TXI: TX interrupt mask
  * @param  NewState: new state of the specified I2C interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_it_config(I2C_TypeDef* I2Cx, uint32_t I2C_IT, bool enabled )
{
  if (enabled) {
    /* Enable the selected I2C interrupts */
    I2Cx->CR1 |= I2C_IT;
  } else {
    /* Disable the selected I2C interrupts */
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_IT);
  }
}

/**
  * @brief  Enables or disables the I2C Clock stretching.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx Clock stretching.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_stretch_clock_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if (enabled )
  {
    /* Enable clock stretching */
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_NOSTRETCH);    
  } else {
    /* Disable clock stretching  */
    I2Cx->CR1 |= I2C_CR1_NOSTRETCH;
  }
}


/**
  * @brief  Enables or disables I2C wakeup from stop mode.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx stop mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_stop_mode_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if ( enabled )
  {
    /* Enable wakeup from stop mode */
    I2Cx->CR1 |= I2C_CR1_WUPEN;   
  } else {
    /* Disable wakeup from stop mode */    
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_WUPEN); 
  }
}

/**
  * @brief  Enables or disables the I2C own address 2.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2C own address 2.
  *          This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
static inline void i2c_dual_address_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if (enabled)
  {
    /* Enable own address 2 */
    I2Cx->OAR2 |= I2C_OAR2_OA2EN;
  } else {
    /* Disable own address 2 */
    I2Cx->OAR2 &= (uint32_t)~((uint32_t)I2C_OAR2_OA2EN);
  }
} 

/**
  * @brief  Configures the I2C slave own address 2 and mask.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Address: specifies the slave address to be programmed.
  * @param  Mask: specifies own address 2 mask to be programmed.
  *          This parameter can be one of the following values:
  *            @arg I2C_OA2_NoMask: no mask.
  *            @arg I2C_OA2_Mask01: OA2[1] is masked and don't care.
  *            @arg I2C_OA2_Mask02: OA2[2:1] are masked and don't care.
  *            @arg I2C_OA2_Mask03: OA2[3:1] are masked and don't care.
  *            @arg I2C_OA2_Mask04: OA2[4:1] are masked and don't care.
  *            @arg I2C_OA2_Mask05: OA2[5:1] are masked and don't care.
  *            @arg I2C_OA2_Mask06: OA2[6:1] are masked and don't care.
  *            @arg I2C_OA2_Mask07: OA2[7:1] are masked and don't care.
  * @retval None
  */
static inline void i2c_own_address2_config(I2C_TypeDef* I2Cx, uint16_t Address, uint8_t Mask)
{
  uint32_t tmpreg = 0;
  
  /* Get the old register value */
  tmpreg = I2Cx->OAR2;

  /* Reset I2Cx OA2 bit [7:1] and OA2MSK bit [1:0]  */
  tmpreg &= (uint32_t)~((uint32_t)(I2C_OAR2_OA2 | I2C_OAR2_OA2MSK));

  /* Set I2Cx SADD */
  tmpreg |= (uint32_t)(((uint32_t)Address & I2C_OAR2_OA2) | \
            (((uint32_t)Mask << 8) & I2C_OAR2_OA2MSK)) ;

  /* Store the new register value */
  I2Cx->OAR2 = tmpreg;
}

/**
  * @brief  Enables or disables the I2C general call mode.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2C general call mode.
  *          This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
static inline void i2c_general_call_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if ( enabled )
  {
    /* Enable general call mode */
    I2Cx->CR1 |= I2C_CR1_GCEN;
  } else {
    /* Disable general call mode */
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_GCEN);
  }
}

/**
  * @brief  Enables or disables the I2C slave byte control.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2C slave byte control.
  *          This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
static inline void i2c_slave_byte_control_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if ( enabled )
  {
    /* Enable slave byte control */
    I2Cx->CR1 |= I2C_CR1_SBC;
  } else {
    /* Disable slave byte control */
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_SBC);
  }
}

/**
  * @brief  Configures the slave address to be transmitted after start generation.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Address: specifies the slave address to be programmed.
  * @note   This function should be called before generating start condition.
  * @retval None
  */
static inline void i2c_slave_address_config(I2C_TypeDef* I2Cx, uint16_t Address)
{
  uint32_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = I2Cx->CR2;

  /* Reset I2Cx SADD bit [9:0] */
  tmpreg &= (uint32_t)~((uint32_t)I2C_CR2_SADD);

  /* Set I2Cx SADD */
  tmpreg |= (uint32_t)((uint32_t)Address & I2C_CR2_SADD);

  /* Store the new register value */
  I2Cx->CR2 = tmpreg;
}

/**
  * @brief  Enables or disables the I2C 10-bit addressing mode for the master.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2C 10-bit addressing mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   This function should be called before generating start condition.
  * @retval None
  */
static inline void i2c_10bit_addressing_mode_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if (enabled)
  {
    /* Enable 10-bit addressing mode */
    I2Cx->CR2 |= I2C_CR2_ADD10;
  } else {
    /* Disable 10-bit addressing mode */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)I2C_CR2_ADD10);
  }
} 

/**
  * @brief  Enables or disables the I2C automatic end mode (stop condition is 
  *         automatically sent when nbytes data are transferred).
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2C automatic end mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   This function has effect if Reload mode is disabled.
  * @retval None
  */
static inline void i2c_auto_end_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if (enabled)
  {
    /* Enable Auto end mode */
    I2Cx->CR2 |= I2C_CR2_AUTOEND;
  } else {
    /* Disable Auto end mode */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)I2C_CR2_AUTOEND);
  }
} 

/**
  * @brief  Enables or disables the I2C nbytes reload mode.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the nbytes reload mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_reload_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if ( enabled )
  {
    /* Enable Auto Reload mode */
    I2Cx->CR2 |= I2C_CR2_RELOAD;
  } else {
    /* Disable Auto Reload mode */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)I2C_CR2_RELOAD);
  }
}

/**
  * @brief  Configures the number of bytes to be transmitted/received.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Number_Bytes: specifies the number of bytes to be programmed.
  * @retval None
  */
static inline void i2c_number_of_bytes_config(I2C_TypeDef* I2Cx, uint8_t Number_Bytes)
{
  uint32_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = I2Cx->CR2;

  /* Reset I2Cx Nbytes bit [7:0] */
  tmpreg &= (uint32_t)~((uint32_t)I2C_CR2_NBYTES);

  /* Set I2Cx Nbytes */
  tmpreg |= (uint32_t)(((uint32_t)Number_Bytes << 16 ) & I2C_CR2_NBYTES);

  /* Store the new register value */
  I2Cx->CR2 = tmpreg;
}  

/**
  * @brief  Configures the type of transfer request for the master.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_Direction: specifies the transfer request direction to be programmed.
  *          This parameter can be one of the following values:
  *            @arg I2C_Direction_Transmitter: Master request a write transfer
  *            @arg I2C_Direction_Receiver: Master request a read transfer 
  * @retval None
  */
static inline void i2c_master_request_config(I2C_TypeDef* I2Cx, uint16_t I2C_Direction)
{
  /* Test on the direction to set/reset the read/write bit */
  if (I2C_Direction == I2C_Direction_Transmitter)
  {
    /* Request a write Transfer */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)I2C_CR2_RD_WRN);
  }
  else
  {
    /* Request a read Transfer */
    I2Cx->CR2 |= I2C_CR2_RD_WRN;
  }
} 

/**
  * @brief  Generates I2Cx communication START condition.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2C START condition generation.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_generate_start(I2C_TypeDef* I2Cx, bool enabled )
{
  
  if ( enabled ) {
    /* Generate a START condition */
    I2Cx->CR2 |= I2C_CR2_START;
  } else {
    /* Disable the START condition generation */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)I2C_CR2_START);
  }
}  

/**
  * @brief  Generates I2Cx communication STOP condition.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2C STOP condition generation.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_generate_stop(I2C_TypeDef* I2Cx, bool enabled )
{
  if ( enabled )
  {
    /* Generate a STOP condition */
    I2Cx->CR2 |= I2C_CR2_STOP;
  } else {
    /* Disable the STOP condition generation */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)I2C_CR2_STOP);
  }
}  

/**
  * @brief  Enables or disables the I2C 10-bit header only mode with read direction.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2C 10-bit header only mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   This mode can be used only when switching from master transmitter mode 
  *         to master receiver mode.
  * @retval None
  */
static inline void i2c_10bit_address_header_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if ( enabled )
  {
    /* Enable 10-bit header only mode */
    I2Cx->CR2 |= I2C_CR2_HEAD10R;
  } else {
    /* Disable 10-bit header only mode */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)I2C_CR2_HEAD10R);
  }
}   

/**
  * @brief  Generates I2C communication Acknowledge.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the Acknowledge.
  *          This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
static inline void i2c_acknowledge_config(I2C_TypeDef* I2Cx, bool enabled)
{
  if ( enabled )
  {
    /* Enable ACK generation */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)I2C_CR2_NACK);    
  } else {
    /* Enable NACK generation */
    I2Cx->CR2 |= I2C_CR2_NACK;
  }
}

/**
  * @brief  Returns the I2C slave matched address .
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @retval The value of the slave matched address .
  */
static inline uint8_t i2c_get_address_matched(I2C_TypeDef* I2Cx)
{
  /* Return the slave matched address in the SR1 register */
  return (uint8_t)(((uint32_t)I2Cx->ISR & I2C_ISR_ADDCODE) >> 16) ;
}

/**
  * @brief  Returns the I2C slave received request.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @retval The value of the received request.
  */
static inline uint16_t i2c_get_transfer_direction(I2C_TypeDef* I2Cx)
{
  uint32_t tmpreg = 0;
  uint16_t direction = 0;
  
  /* Return the slave matched address in the SR1 register */
  tmpreg = (uint32_t)(I2Cx->ISR & I2C_ISR_DIR);
  
  /* If write transfer is requested */
  if (tmpreg == 0)
  {
    /* write transfer is requested */
    direction = I2C_Direction_Transmitter;
  }
  else
  {
    /* Read transfer is requested */
    direction = I2C_Direction_Receiver;
  }  
  return direction;
}

/**
  * @brief  Handles I2Cx communication when starting transfer or during transfer (TC or TCR flag are set).
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Address: specifies the slave address to be programmed.
  * @param  Number_Bytes: specifies the number of bytes to be programmed.
  *          This parameter must be a value between 0 and 255.
  * @param  ReloadEndMode: new state of the I2C START condition generation.
  *          This parameter can be one of the following values:
  *            @arg I2C_Reload_Mode: Enable Reload mode .
  *            @arg I2C_AutoEnd_Mode: Enable Automatic end mode.
  *            @arg I2C_SoftEnd_Mode: Enable Software end mode.
  * @param  StartStopMode: new state of the I2C START condition generation.
  *          This parameter can be one of the following values:
  *            @arg I2C_No_StartStop: Don't Generate stop and start condition.
  *            @arg I2C_Generate_Stop: Generate stop condition (Number_Bytes should be set to 0).
  *            @arg I2C_Generate_Start_Read: Generate Restart for read request.
  *            @arg I2C_Generate_Start_Write: Generate Restart for write request.
  * @retval None
  */
static inline void i2c_transfer_handling(I2C_TypeDef* I2Cx, uint16_t Address, uint8_t Number_Bytes, uint32_t ReloadEndMode, uint32_t StartStopMode)
{
  uint32_t tmpreg = 0;
    
  /* Get the CR2 register value */
  tmpreg = I2Cx->CR2;
  
  /* clear tmpreg specific bits */
  tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
  
  /* update tmpreg */
  tmpreg |= (uint32_t)(((uint32_t)Address & I2C_CR2_SADD) | (((uint32_t)Number_Bytes << 16 ) & I2C_CR2_NBYTES) | (uint32_t)ReloadEndMode | (uint32_t)StartStopMode);
  
  /* update CR2 register */
  I2Cx->CR2 = tmpreg;  
}  


// SMBUS functionality

/**
  * @brief  Enables or disables I2C SMBus alert.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx SMBus alert.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_smbus_alert_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if (enabled )
  {
    /* Enable SMBus alert */
    I2Cx->CR1 |= I2C_CR1_ALERTEN;   
  } else {
    /* Disable SMBus alert */    
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_ALERTEN); 
  }
}

/**
  * @brief  Enables or disables I2C Clock Timeout (SCL Timeout detection).
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx clock Timeout.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_clock_timeout_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if ( enabled )
  {
    /* Enable Clock Timeout */
    I2Cx->TIMEOUTR |= I2C_TIMEOUTR_TIMOUTEN;   
  } else {
    /* Disable Clock Timeout */    
    I2Cx->TIMEOUTR &= (uint32_t)~((uint32_t)I2C_TIMEOUTR_TIMOUTEN); 
  }
}

/**
  * @brief  Enables or disables I2C Extended Clock Timeout (SCL cumulative Timeout detection).
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx Extended clock Timeout.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_extended_clock_timeout_cmd(I2C_TypeDef* I2Cx, bool enabled)
{
  if (enabled)
  {
    /* Enable Clock Timeout */
    I2Cx->TIMEOUTR |= I2C_TIMEOUTR_TEXTEN;   
  } else {
    /* Disable Clock Timeout */    
    I2Cx->TIMEOUTR &= (uint32_t)~((uint32_t)I2C_TIMEOUTR_TEXTEN); 
  }
}


/**
  * @brief  Enables or disables I2C Idle Clock Timeout (Bus idle SCL and SDA 
  *         high detection).
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx Idle clock Timeout.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_idle_clock_timeout_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if (enabled)
  {
    /* Enable Clock Timeout */
    I2Cx->TIMEOUTR |= I2C_TIMEOUTR_TIDLE;   
  }
  else
  {
    /* Disable Clock Timeout */    
    I2Cx->TIMEOUTR &= (uint32_t)~((uint32_t)I2C_TIMEOUTR_TIDLE); 
  }
}

/**
  * @brief  Configures the I2C Bus Timeout A (SCL Timeout when TIDLE = 0 or Bus 
  *         idle SCL and SDA high when TIDLE = 1).
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Timeout: specifies the TimeoutA to be programmed. 
  * @retval None
  */
static inline void i2c_timeout_a_config(I2C_TypeDef* I2Cx, uint16_t Timeout)
{
  uint32_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = I2Cx->TIMEOUTR;

  /* Reset I2Cx TIMEOUTA bit [11:0] */
  tmpreg &= (uint32_t)~((uint32_t)I2C_TIMEOUTR_TIMEOUTA);

  /* Set I2Cx TIMEOUTA */
  tmpreg |= (uint32_t)((uint32_t)Timeout & I2C_TIMEOUTR_TIMEOUTA) ;

  /* Store the new register value */
  I2Cx->TIMEOUTR = tmpreg;
}

/**
  * @brief  Configures the I2C Bus Timeout B (SCL cumulative Timeout).
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Timeout: specifies the TimeoutB to be programmed. 
  * @retval None
  */
static inline void i2c_timeout_b_config(I2C_TypeDef* I2Cx, uint16_t Timeout)
{
  uint32_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = I2Cx->TIMEOUTR;

  /* Reset I2Cx TIMEOUTB bit [11:0] */
  tmpreg &= (uint32_t)~((uint32_t)I2C_TIMEOUTR_TIMEOUTB);

  /* Set I2Cx TIMEOUTB */
  tmpreg |= (uint32_t)(((uint32_t)Timeout << 16) & I2C_TIMEOUTR_TIMEOUTB) ;

  /* Store the new register value */
  I2Cx->TIMEOUTR = tmpreg;
}

/**
  * @brief  Enables or disables I2C PEC calculation.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx PEC calculation.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_calculate_pec(I2C_TypeDef* I2Cx, bool enabled)
{
  if ( enabled )
  {
    /* Enable PEC calculation */
    I2Cx->CR1 |= I2C_CR1_PECEN;   
  } else {
    /* Disable PEC calculation */    
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_PECEN); 
  }
}

/**
  * @brief  Enables or disables I2C PEC transmission/reception request.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx PEC request.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_pec_request_cmd(I2C_TypeDef* I2Cx, bool enabled )
{
  if ( enabled )
  {
    /* Enable PEC transmission/reception request */
    I2Cx->CR1 |= I2C_CR2_PECBYTE;   
  } else {
    /* Disable PEC transmission/reception request */    
    I2Cx->CR1 &= (uint32_t)~((uint32_t)I2C_CR2_PECBYTE); 
  }
}


/**
  * @brief  Returns the I2C PEC.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @retval The value of the PEC .
  */
static inline uint8_t i2c_get_pec(I2C_TypeDef* I2Cx)
{
  /* Return the slave matched address in the SR1 register */
  return (uint8_t)((uint32_t)I2Cx->PECR & I2C_PECR_PEC);
}


  /**
  * @brief  Reads the specified I2C register and returns its value.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_Register: specifies the register to read.
  *          This parameter can be one of the following values:
  *            @arg I2C_Register_CR1: CR1 register.
  *            @arg I2C_Register_CR2: CR2 register.
  *            @arg I2C_Register_OAR1: OAR1 register.
  *            @arg I2C_Register_OAR2: OAR2 register.
  *            @arg I2C_Register_TIMINGR: TIMING register.
  *            @arg I2C_Register_TIMEOUTR: TIMEOUTR register.
  *            @arg I2C_Register_ISR: ISR register.
  *            @arg I2C_Register_ICR: ICR register.
  *            @arg I2C_Register_PECR: PECR register.
  *            @arg I2C_Register_RXDR: RXDR register.
  *            @arg I2C_Register_TXDR: TXDR register.
  * @retval The value of the read register.
  */
static inline uint32_t I2C_ReadRegister(I2C_TypeDef* I2Cx, uint8_t I2C_Register)
{
  __IO uint32_t tmp = 0;

  tmp = (uint32_t)I2Cx;
  tmp += I2C_Register;

  /* Return the selected register value */
  return (*(__IO uint32_t *) tmp);
}

/**
  * @brief  Sends a data byte through the I2Cx peripheral.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Data: Byte to be transmitted..
  * @retval None
  */
static inline void i2c_send_data(I2C_TypeDef* I2Cx, uint8_t Data)
{
  /* Write in the DR register the data to be sent */
  I2Cx->TXDR = (uint8_t)Data;
}

/**
  * @brief  Returns the most recent received data by the I2Cx peripheral.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @retval The value of the received data.
  */
static inline uint8_t i2c_receive_data(I2C_TypeDef* I2Cx)
{
  /* Return the data in the DR register */
  return (uint8_t)I2Cx->RXDR;
}  

/**
  * @brief  Enables or disables the I2C DMA interface.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_DMAReq: specifies the I2C DMA transfer request to be enabled or disabled. 
  *          This parameter can be any combination of the following values:
  *            @arg I2C_DMAReq_Tx: Tx DMA transfer request
  *            @arg I2C_DMAReq_Rx: Rx DMA transfer request
  * @param  NewState: new state of the selected I2C DMA transfer request.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2c_dma_cmd(I2C_TypeDef* I2Cx, uint32_t I2C_DMAReq, bool enabled )
{
  if ( enabled )
  {
    /* Enable the selected I2C DMA requests */
    I2Cx->CR1 |= I2C_DMAReq;
  } else {
    /* Disable the selected I2C DMA requests */
    I2Cx->CR1 &= (uint32_t)~I2C_DMAReq;
  }
}

/**
  * @brief  Checks whether the specified I2C flag is set or not.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_FLAG: specifies the flag to check. 
  *          This parameter can be one of the following values:
  *            @arg I2C_FLAG_TXE: Transmit data register empty
  *            @arg I2C_FLAG_TXIS: Transmit interrupt status
  *            @arg I2C_FLAG_RXNE: Receive data register not empty
  *            @arg I2C_FLAG_ADDR: Address matched (slave mode)
  *            @arg I2C_FLAG_NACKF: NACK received flag
  *            @arg I2C_FLAG_STOPF: STOP detection flag
  *            @arg I2C_FLAG_TC: Transfer complete (master mode)
  *            @arg I2C_FLAG_TCR: Transfer complete reload
  *            @arg I2C_FLAG_BERR: Bus error
  *            @arg I2C_FLAG_ARLO: Arbitration lost
  *            @arg I2C_FLAG_OVR: Overrun/Underrun
  *            @arg I2C_FLAG_PECERR: PEC error in reception
  *            @arg I2C_FLAG_TIMEOUT: Timeout or Tlow detection flag
  *            @arg I2C_FLAG_ALERT: SMBus Alert
  *            @arg I2C_FLAG_BUSY: Bus busy
  * @retval The new state of I2C_FLAG (SET or RESET).
  */
static inline bool  i2c_get_flag_status(I2C_TypeDef* I2Cx, uint32_t I2C_FLAG)
{
  /* Get the ISR register value */
  uint32_t tmpreg = I2Cx->ISR;
  
  /* Get flag status */
  tmpreg &= I2C_FLAG;
  
  return (tmpreg != 0);
} 


/**
  * @brief  Clears the I2Cx's pending flags.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_FLAG: specifies the flag to clear. 
  *          This parameter can be any combination of the following values:
  *            @arg I2C_FLAG_ADDR: Address matched (slave mode)
  *            @arg I2C_FLAG_NACKF: NACK received flag
  *            @arg I2C_FLAG_STOPF: STOP detection flag
  *            @arg I2C_FLAG_BERR: Bus error
  *            @arg I2C_FLAG_ARLO: Arbitration lost
  *            @arg I2C_FLAG_OVR: Overrun/Underrun
  *            @arg I2C_FLAG_PECERR: PEC error in reception
  *            @arg I2C_FLAG_TIMEOUT: Timeout or Tlow detection flag
  *            @arg I2C_FLAG_ALERT: SMBus Alert
  * @retval The new state of I2C_FLAG (SET or RESET).
  */
static inline void i2c_clear_flag(I2C_TypeDef* I2Cx, uint32_t I2C_FLAG)
{ 
  /* Clear the selected flag */
  I2Cx->ICR = I2C_FLAG;
}


/**
  * @brief  Checks whether the specified I2C interrupt has occurred or not.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_IT: specifies the interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg I2C_IT_TXIS: Transmit interrupt status
  *            @arg I2C_IT_RXNE: Receive data register not empty
  *            @arg I2C_IT_ADDR: Address matched (slave mode)
  *            @arg I2C_IT_NACKF: NACK received flag
  *            @arg I2C_IT_STOPF: STOP detection flag
  *            @arg I2C_IT_TC: Transfer complete (master mode)
  *            @arg I2C_IT_TCR: Transfer complete reload
  *            @arg I2C_IT_BERR: Bus error
  *            @arg I2C_IT_ARLO: Arbitration lost
  *            @arg I2C_IT_OVR: Overrun/Underrun
  *            @arg I2C_IT_PECERR: PEC error in reception
  *            @arg I2C_IT_TIMEOUT: Timeout or Tlow detection flag
  *            @arg I2C_IT_ALERT: SMBus Alert
  * @retval The new state of I2C_IT (SET or RESET).
  */
static inline bool I2C_GetITStatus(I2C_TypeDef* I2Cx, uint32_t I2C_IT)
{
  uint32_t tmpreg = 0;
  uint32_t enablestatus = 0;


  /* Check if the interrupt source is enabled or not */
  /* If Error interrupt */
  if ((uint32_t)(I2C_IT & ERROR_IT_MASK))
  {
    enablestatus = (uint32_t)((I2C_CR1_ERRIE) & (I2Cx->CR1));
  }
  /* If TC interrupt */
  else if ((uint32_t)(I2C_IT & TC_IT_MASK))
  {
    enablestatus = (uint32_t)((I2C_CR1_TCIE) & (I2Cx->CR1));
  }
  else
  {
    enablestatus = (uint32_t)((I2C_IT) & (I2Cx->CR1));
  }
  
  /* Get the ISR register value */
  tmpreg = I2Cx->ISR;

  /* Get flag status */
  tmpreg &= I2C_IT;

  /* Check the status of the specified I2C flag */
  return((tmpreg != RESET) && enablestatus);
}


/**
  * @brief  Clears the I2Cx's interrupt pending bits.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  I2C_IT: specifies the interrupt pending bit to clear.
  *          This parameter can be any combination of the following values:
  *            @arg I2C_IT_ADDR: Address matched (slave mode)
  *            @arg I2C_IT_NACKF: NACK received flag
  *            @arg I2C_IT_STOPF: STOP detection flag
  *            @arg I2C_IT_BERR: Bus error
  *            @arg I2C_IT_ARLO: Arbitration lost
  *            @arg I2C_IT_OVR: Overrun/Underrun
  *            @arg I2C_IT_PECERR: PEC error in reception
  *            @arg I2C_IT_TIMEOUT: Timeout or Tlow detection flag
  *            @arg I2C_IT_ALERT: SMBus Alert
  * @retval The new state of I2C_IT (SET or RESET).
  */
static inline void i2c_clear_it_pending_bit(I2C_TypeDef* I2Cx, uint32_t I2C_IT)
{
  /* Clear the selected flag */
  I2Cx->ICR = I2C_IT;
}

#ifdef __cplusplus
}
#endif

#undef CR1_CLEAR_MASK   
#undef CR2_CLEAR_MASK   
#undef TIMING_CLEAR_MASK
#undef ERROR_IT_MASK    
#undef TC_IT_MASK       

