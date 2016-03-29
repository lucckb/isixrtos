
/*
 * stm32i2c.h
 *
 *  Created on: 23-07-2012
 *      Author: lucck
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "stm32f1f2f4_i2c.h"
#include "stm32lib.h"
#include "stm32rcc.h"


#define CR1_CLEAR_MASK    ((uint16_t)0xFBF5)
#define FLAG_MASK         ((uint32_t)0x00FFFFFF)  /*<! I2C FLAG mask */
#define ITEN_MASK         ((uint32_t)0x07000000)  /*<! I2C Interrupt Enable mask */

#ifdef __cplusplus
namespace stm32 {
#endif


/**
  * @brief  Deinitialize the I2Cx peripheral registers to their default reset values.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @retval None
  */
static inline void i2c_deinit(I2C_TypeDef* I2Cx)
{

  if (I2Cx == I2C1)
  {
    /* Enable I2C1 reset state */
    rcc_apb1_periph_reset_cmd(RCC_APB1Periph_I2C1, true);
    /* Release I2C1 from reset state */
    rcc_apb1_periph_reset_cmd(RCC_APB1Periph_I2C1, false);
  }
  else if (I2Cx == I2C2)
  {
    /* Enable I2C2 reset state */
	rcc_apb1_periph_reset_cmd(RCC_APB1Periph_I2C2, true);
    /* Release I2C2 from reset state */
	rcc_apb1_periph_reset_cmd(RCC_APB1Periph_I2C2, false);
  }
#if defined(STM32MCU_MAJOR_TYPE_F2) || defined(STM32MCU_MAJOR_TYPE_F4)
  else
  {
    if (I2Cx == I2C3)
    {
      /* Enable I2C3 reset state */
      rcc_apb1_periph_reset_cmd(RCC_APB1Periph_I2C3, true);
      /* Release I2C3 from reset state */
      rcc_apb1_periph_reset_cmd(RCC_APB1Periph_I2C3, false);
    }
  }
#endif
}


static inline void i2c_init(I2C_TypeDef* I2Cx, uint32_t clock_speed, uint16_t mode, uint16_t duty_cycle,
		uint16_t own_address, uint16_t ack, uint16_t acknowledged_address, uint32_t pclk1 )
{
  uint16_t tmpreg = 0, freqrange = 0;
  uint16_t result = 0x04;


  /* Get the I2Cx CR2 value */
  tmpreg = I2Cx->CR2;
  /* Clear frequency FREQ[5:0] bits */
  tmpreg &= (uint16_t)~((uint16_t)I2C_CR2_FREQ);
  /* Set frequency bits depending on pclk1 value */
  freqrange = (uint16_t)(pclk1 / 1000000);
  tmpreg |= freqrange;
  /* Write to I2Cx CR2 */
  I2Cx->CR2 = tmpreg;


  /* Disable the selected I2C peripheral to configure TRISE */
  I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_PE);
  /* Reset tmpreg value */
  /* Clear F/S, DUTY and CCR[11:0] bits */
  tmpreg = 0;

  /* Configure speed in standard mode */
  if (clock_speed <= 100000)
  {
    /* Standard mode speed calculate */
    result = (uint16_t)(pclk1 / (clock_speed << 1));
    /* Test if CCR value is under 0x4*/
    if (result < 0x04)
    {
      /* Set minimum allowed value */
      result = 0x04;
    }
    /* Set speed value for standard mode */
    tmpreg |= result;
    /* Set Maximum Rise Time for standard mode */
    I2Cx->TRISE = freqrange + 1;
  }
  /* Configure speed in fast mode */
  /* To use the I2C at 400 KHz (in fast mode), the PCLK1 frequency (I2C peripheral
     input clock) must be a multiple of 10 MHz */
  else /*(I2C_InitStruct->I2C_ClockSpeed <= 400000)*/
  {
    if (duty_cycle == I2C_DutyCycle_2)
    {
      /* Fast mode speed calculate: Tlow/Thigh = 2 */
      result = (uint16_t)(pclk1 / (clock_speed * 3));
    }
    else /*I2C_InitStruct->I2C_DutyCycle == I2C_DutyCycle_16_9*/
    {
      /* Fast mode speed calculate: Tlow/Thigh = 16/9 */
      result = (uint16_t)(pclk1 / (clock_speed * 25));
      /* Set DUTY bit */
      result |= I2C_DutyCycle_16_9;
    }

    /* Test if CCR value is under 0x1*/
    if ((result & I2C_CCR_CCR) == 0)
    {
      /* Set minimum allowed value */
      result |= (uint16_t)0x0001;
    }
    /* Set speed value and set F/S bit for fast mode */
    tmpreg |= (uint16_t)(result | I2C_CCR_FS);
    /* Set Maximum Rise Time for fast mode */
    I2Cx->TRISE = (uint16_t)(((freqrange * (uint16_t)300) / (uint16_t)1000) + (uint16_t)1);
  }

  /* Write to I2Cx CCR */
  I2Cx->CCR = tmpreg;
  /* Enable the selected I2C peripheral */
  I2Cx->CR1 |= I2C_CR1_PE;


  /* Get the I2Cx CR1 value */
  tmpreg = I2Cx->CR1;
  /* Clear ACK, SMBTYPE and  SMBUS bits */
  tmpreg &= CR1_CLEAR_MASK;
  /* Configure I2Cx: mode and acknowledgement */
  /* Set SMBTYPE and SMBUS bits according to I2C_Mode value */
  /* Set ACK bit according to I2C_Ack value */
  tmpreg |= (uint16_t)((uint32_t)mode | ack);
  /* Write to I2Cx CR1 */
  I2Cx->CR1 = tmpreg;


  /* Set I2Cx Own Address1 and acknowledged address */
  I2Cx->OAR1 = (acknowledged_address | own_address);
}

/**
  * @brief  Enables or disables the specified I2C peripheral.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx peripheral.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_cmd(I2C_TypeDef* I2Cx, bool en)
{
  /* Check the parameters */
  if (en)
  {
    /* Enable the selected I2C peripheral */
    I2Cx->CR1 |= I2C_CR1_PE;
  }
  else
  {
    /* Disable the selected I2C peripheral */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_PE);
  }
}


/**
  * @brief  Generates I2Cx communication START condition.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C START condition generation.
  *          This parameter can be: true or false.
  * @retval None.
  */
static inline void i2c_generate_start(I2C_TypeDef* I2Cx, bool en)
{
  if (en)
  {
    /* Generate a START condition */
    I2Cx->CR1 |= I2C_CR1_START;
  }
  else
  {
    /* Disable the START condition generation */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_START);
  }
}

/**
  * @brief  Generates I2Cx communication STOP condition.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C STOP condition generation.
  *          This parameter can be: true or false.
  * @retval None.
  */
static inline void i2c_generate_stop(I2C_TypeDef* I2Cx, bool en)
{
  if (en)
  {
    /* Generate a STOP condition */
    I2Cx->CR1 |= I2C_CR1_STOP;
  }
  else
  {
    /* Disable the STOP condition generation */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_STOP);
  }
}

/**
  * @brief  Transmits the address byte to select the slave device.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  Address: specifies the slave address which will be transmitted
  * @param  I2C_Direction: specifies whether the I2C device will be a Transmitter
  *         or a Receiver.
  *          This parameter can be one of the following values
  *            @arg I2C_Direction_Transmitter: Transmitter mode
  *            @arg I2C_Direction_Receiver: Receiver mode
  * @retval None.
  */
static inline void i2c_send_7bit_address(I2C_TypeDef* I2Cx, uint8_t Address, uint8_t I2C_Direction)
{
  /* Test on the direction to set/reset the read/write bit */
  if (I2C_Direction != I2C_Direction_Transmitter)
  {
    /* Set the address bit0 for read */
    Address |= I2C_OAR1_ADD0;
  }
  else
  {
    /* Reset the address bit0 for write */
    Address &= (uint8_t)~((uint8_t)I2C_OAR1_ADD0);
  }
  /* Send the address */
  I2Cx->DR = Address;
}

/**
 * Send 7 bit full address
 * @param I2Cx I2C periph
 * @param Address Full address without modify RW bit
 */
static inline void i2c_send_f7bit_address(I2C_TypeDef* I2Cx, uint8_t Address)
{
	I2Cx->DR = Address;
}


/**
  * @brief  Enables or disables the specified I2C acknowledge feature.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C Acknowledgement.
  *          This parameter can be: true or false.
  * @retval None.
  */
static inline void i2c_acknowledge_config(I2C_TypeDef* I2Cx, bool en)
{
  if ( en )
  {
    /* Enable the acknowledgement */
    I2Cx->CR1 |= I2C_CR1_ACK;
  }
  else
  {
    /* Disable the acknowledgement */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
  }
}

/**
  * @brief  Configures the specified I2C own address2.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  Address: specifies the 7bit I2C own address2.
  * @retval None.
  */
static inline void i2c_own_address2config(I2C_TypeDef* I2Cx, uint8_t Address)
{
  uint16_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = I2Cx->OAR2;

  /* Reset I2Cx Own address2 bit [7:1] */
  tmpreg &= (uint16_t)~((uint16_t)I2C_OAR2_ADD2);

  /* Set I2Cx Own address2 */
  tmpreg |= (uint16_t)((uint16_t)Address & (uint16_t)0x00FE);

  /* Store the new register value */
  I2Cx->OAR2 = tmpreg;
}

/**
  * @brief  Enables or disables the specified I2C dual addressing mode.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C dual addressing mode.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_dual_address_cmd(I2C_TypeDef* I2Cx, bool NewState)
{
  if (NewState != false)
  {
    /* Enable dual addressing mode */
    I2Cx->OAR2 |= I2C_OAR2_ENDUAL;
  }
  else
  {
    /* Disable dual addressing mode */
    I2Cx->OAR2 &= (uint16_t)~((uint16_t)I2C_OAR2_ENDUAL);
  }
}

/**
  * @brief  Enables or disables the specified I2C general call feature.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C General call.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_general_call_cmd(I2C_TypeDef* I2Cx, bool NewState)
{

  if (NewState != false)
  {
    /* Enable generall call */
    I2Cx->CR1 |= I2C_CR1_ENGC;
  }
  else
  {
    /* Disable generall call */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ENGC);
  }
}

/**
  * @brief  Enables or disables the specified I2C software reset.
  * @note   When software reset is enabled, the I2C IOs are released (this can
  *         be useful to recover from bus errors).
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C software reset.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_software_reset_cmd(I2C_TypeDef* I2Cx, bool NewState)
{
  if (NewState != false)
  {
    /* Peripheral under reset */
    I2Cx->CR1 |= I2C_CR1_SWRST;
  }
  else
  {
    /* Peripheral not under reset */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_SWRST);
  }
}

/**
  * @brief  Enables or disables the specified I2C Clock stretching.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx Clock stretching.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_stretch_clock_cmd(I2C_TypeDef* I2Cx, bool NewState)
{
  if (NewState == false)
  {
    /* Enable the selected I2C Clock stretching */
    I2Cx->CR1 |= I2C_CR1_NOSTRETCH;
  }
  else
  {
    /* Disable the selected I2C Clock stretching */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_NOSTRETCH);
  }
}

/**
  * @brief  Selects the specified I2C fast mode duty cycle.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_DutyCycle: specifies the fast mode duty cycle.
  *          This parameter can be one of the following values:
  *            @arg I2C_DutyCycle_2: I2C fast mode Tlow/Thigh = 2
  *            @arg I2C_DutyCycle_16_9: I2C fast mode Tlow/Thigh = 16/9
  * @retval None
  */
static inline void i2c_fast_mode_duty_cycle_config(I2C_TypeDef* I2Cx, uint16_t I2C_DutyCycle)
{
  if (I2C_DutyCycle != I2C_DutyCycle_16_9)
  {
    /* I2C fast mode Tlow/Thigh=2 */
    I2Cx->CCR &= I2C_DutyCycle_2;
  }
  else
  {
    /* I2C fast mode Tlow/Thigh=16/9 */
    I2Cx->CCR |= I2C_DutyCycle_16_9;
  }
}

/**
  * @brief  Selects the specified I2C NACK position in master receiver mode.
  * @note   This function is useful in I2C Master Receiver mode when the number
  *         of data to be received is equal to 2. In this case, this function
  *         should be called (with parameter I2C_NACKPosition_Next) before data
  *         reception starts,as described in the 2-byte reception procedure
  *         recommended in Reference Manual in Section: Master receiver.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_NACKPosition: specifies the NACK position.
  *          This parameter can be one of the following values:
  *            @arg I2C_NACKPosition_Next: indicates that the next byte will be the last
  *                                        received byte.
  *            @arg I2C_NACKPosition_Current: indicates that current byte is the last
  *                                           received byte.
  *
  * @note    This function configures the same bit (POS) as I2C_PECPositionConfig()
  *          but is intended to be used in I2C mode while I2C_PECPositionConfig()
  *          is intended to used in SMBUS mode.
  *
  * @retval None
  */
static inline void i2c_nack_position_config(I2C_TypeDef* I2Cx, uint16_t I2C_NACKPosition)
{
  /* Check the input parameter */
  if (I2C_NACKPosition == I2C_NACKPosition_Next)
  {
    /* Next byte in shift register is the last received byte */
    I2Cx->CR1 |= I2C_NACKPosition_Next;
  }
  else
  {
    /* Current byte in shift register is the last received byte */
    I2Cx->CR1 &= I2C_NACKPosition_Current;
  }
}

/**
  * @brief  Drives the SMBusAlert pin high or low for the specified I2C.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_SMBusAlert: specifies SMBAlert pin level.
  *          This parameter can be one of the following values:
  *            @arg I2C_SMBusAlert_Low: SMBAlert pin driven low
  *            @arg I2C_SMBusAlert_High: SMBAlert pin driven high
  * @retval None
  */
static inline void i2c_smbus_alert_config(I2C_TypeDef* I2Cx, uint16_t I2C_SMBusAlert)
{
  if (I2C_SMBusAlert == I2C_SMBusAlert_Low)
  {
    /* Drive the SMBusAlert pin Low */
    I2Cx->CR1 |= I2C_SMBusAlert_Low;
  }
  else
  {
    /* Drive the SMBusAlert pin High  */
    I2Cx->CR1 &= I2C_SMBusAlert_High;
  }
}

/**
  * @brief  Enables or disables the specified I2C ARP.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx ARP.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_arp_cmd(I2C_TypeDef* I2Cx, bool NewState)
{
  if (NewState != false)
  {
    /* Enable the selected I2C ARP */
    I2Cx->CR1 |= I2C_CR1_ENARP;
  }
  else
  {
    /* Disable the selected I2C ARP */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ENARP);
  }
}

/**
  * @brief  Sends a data byte through the I2Cx peripheral.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  Data: Byte to be transmitted..
  * @retval None
  */
static inline void i2c_send_data(I2C_TypeDef* I2Cx, uint8_t Data)
{
  /* Write in the DR register the data to be sent */
  I2Cx->DR = Data;
}

/**
  * @brief  Returns the most recent received data by the I2Cx peripheral.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @retval The value of the received data.
  */
static inline uint8_t i2c_receive_data(I2C_TypeDef* I2Cx)
{
  /* Return the data in the DR register */
  return (uint8_t)I2Cx->DR;
}

/**
  * @brief  Enables or disables the specified I2C PEC transfer.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C PEC transmission.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_transmit_pec(I2C_TypeDef* I2Cx, bool NewState)
{
  if (NewState != false)
  {
    /* Enable the selected I2C PEC transmission */
    I2Cx->CR1 |= I2C_CR1_PEC;
  }
  else
  {
    /* Disable the selected I2C PEC transmission */
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_PEC);
  }
}

/**
  * @brief  Selects the specified I2C PEC position.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_PECPosition: specifies the PEC position.
  *          This parameter can be one of the following values:
  *            @arg I2C_PECPosition_Next: indicates that the next byte is PEC
  *            @arg I2C_PECPosition_Current: indicates that current byte is PEC
  *
  * @note    This function configures the same bit (POS) as I2C_NACKPositionConfig()
  *          but is intended to be used in SMBUS mode while I2C_NACKPositionConfig()
  *          is intended to used in I2C mode.
  *
  * @retval None
  */
static inline void i2c_pec_position_config(I2C_TypeDef* I2Cx, uint16_t I2C_PECPosition)
{
  if (I2C_PECPosition == I2C_PECPosition_Next)
  {
    /* Next byte in shift register is PEC */
    I2Cx->CR1 |= I2C_PECPosition_Next;
  }
  else
  {
    /* Current byte in shift register is PEC */
    I2Cx->CR1 &= I2C_PECPosition_Current;
  }
}

/**
  * @brief  Returns the PEC value for the specified I2C.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @retval The PEC value.
  */
static inline uint8_t i2c_get_pec(I2C_TypeDef* I2Cx)
{
  /* Return the selected I2C PEC value */
  return ((I2Cx->SR2) >> 8);
}

/**
  * @brief  Enables or disables the specified I2C DMA requests.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C DMA transfer.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_dma_cmd(I2C_TypeDef* I2Cx, bool en)
{
  if ( en )
  {
    /* Enable the selected I2C DMA requests */
    I2Cx->CR2 |= I2C_CR2_DMAEN;
  }
  else
  {
    /* Disable the selected I2C DMA requests */
    I2Cx->CR2 &= (uint16_t)~((uint16_t)I2C_CR2_DMAEN);
  }
}

/**
  * @brief  Specifies that the next DMA transfer is the last one.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C DMA last transfer.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_dma_last_transfer_cmd(I2C_TypeDef* I2Cx, bool NewState)
{
  if (NewState != false)
  {
    /* Next DMA transfer is the last transfer */
    I2Cx->CR2 |= I2C_CR2_LAST;
  }
  else
  {
    /* Next DMA transfer is not the last transfer */
    I2Cx->CR2 &= (uint16_t)~((uint16_t)I2C_CR2_LAST);
  }
}

/**
  * @brief  Reads the specified I2C register and returns its value.
  * @param  I2C_Register: specifies the register to read.
  *          This parameter can be one of the following values:
  *            @arg I2C_Register_CR1:  CR1 register.
  *            @arg I2C_Register_CR2:   CR2 register.
  *            @arg I2C_Register_OAR1:  OAR1 register.
  *            @arg I2C_Register_OAR2:  OAR2 register.
  *            @arg I2C_Register_DR:    DR register.
  *            @arg I2C_Register_SR1:   SR1 register.
  *            @arg I2C_Register_SR2:   SR2 register.
  *            @arg I2C_Register_CCR:   CCR register.
  *            @arg I2C_Register_TRISE: TRISE register.
  * @retval The value of the read register.
  */
static inline uint16_t i2c_read_register(I2C_TypeDef* I2Cx, uint8_t I2C_Register)
{
  __IO uint32_t tmp;


  tmp = (uint32_t) I2Cx;
  tmp += I2C_Register;

  /* Return the selected register value */
  return (*(__IO uint16_t *) tmp);
}


/**
  * @brief  Enables or disables the specified I2C interrupts.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_IT: specifies the I2C interrupts sources to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *            @arg I2C_IT_BUF: Buffer interrupt mask
  *            @arg I2C_IT_EVT: Event interrupt mask
  *            @arg I2C_IT_ERR: Error interrupt mask
  * @param  NewState: new state of the specified I2C interrupts.
  *          This parameter can be: true or false.
  * @retval None
  */
static inline void i2c_it_config(I2C_TypeDef* I2Cx, uint16_t I2C_IT, bool en )
{
  if ( en )
  {
    /* Enable the selected I2C interrupts */
    I2Cx->CR2 |= I2C_IT;
  }
  else
  {
    /* Disable the selected I2C interrupts */
    I2Cx->CR2 &= (uint16_t)~I2C_IT;
  }
}

/**
  * @brief  Checks whether the last I2Cx Event is equal to the one passed
  *         as parameter.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_EVENT: specifies the event to be checked.
  *          This parameter can be one of the following values:
  *            @arg I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED: EV1
  *            @arg I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED: EV1
  *            @arg I2C_EVENT_SLAVE_TRANSMITTER_SECONDADDRESS_MATCHED: EV1
  *            @arg I2C_EVENT_SLAVE_RECEIVER_SECONDADDRESS_MATCHED: EV1
  *            @arg I2C_EVENT_SLAVE_GENERALCALLADDRESS_MATCHED: EV1
  *            @arg I2C_EVENT_SLAVE_BYTE_RECEIVED: EV2
  *            @arg (I2C_EVENT_SLAVE_BYTE_RECEIVED | I2C_FLAG_DUALF): EV2
  *            @arg (I2C_EVENT_SLAVE_BYTE_RECEIVED | I2C_FLAG_GENCALL): EV2
  *            @arg I2C_EVENT_SLAVE_BYTE_TRANSMITTED: EV3
  *            @arg (I2C_EVENT_SLAVE_BYTE_TRANSMITTED | I2C_FLAG_DUALF): EV3
  *            @arg (I2C_EVENT_SLAVE_BYTE_TRANSMITTED | I2C_FLAG_GENCALL): EV3
  *            @arg I2C_EVENT_SLAVE_ACK_FAILURE: EV3_2
  *            @arg I2C_EVENT_SLAVE_STOP_DETECTED: EV4
  *            @arg I2C_EVENT_MASTER_MODE_SELECT: EV5
  *            @arg I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED: EV6
  *            @arg I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED: EV6
  *            @arg I2C_EVENT_MASTER_BYTE_RECEIVED: EV7
  *            @arg I2C_EVENT_MASTER_BYTE_TRANSMITTING: EV8
  *            @arg I2C_EVENT_MASTER_BYTE_TRANSMITTED: EV8_2
  *            @arg I2C_EVENT_MASTER_MODE_ADDRESS10: EV9
  *
  * @note   For detailed description of Events, please refer to section I2C_Events
  *         in stm32f4xx_i2c.h file.
  *
  * @retval An bool enumeration value:
  *           - true: Last event is equal to the I2C_EVENT
  *           - false: Last event is different from the I2C_EVENT
  */
static inline bool i2c_check_event(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
  uint32_t lastevent;
  uint32_t flag1, flag2;

  /* Read the I2Cx status register */
  flag1 = I2Cx->SR1;
  flag2 = I2Cx->SR2;
  flag2 = flag2 << 16;

  /* Get the last event value from I2C status register */
  lastevent = (flag1 | flag2) & FLAG_MASK;

  /* Check whether the last event contains the I2C_EVENT */
  return ((lastevent & I2C_EVENT) == I2C_EVENT)?true:false;

}

/**
  * @brief  Returns the last I2Cx Event.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  *
  * @note   For detailed description of Events, please refer to section I2C_Events
  *         in stm32f4xx_i2c.h file.
  *
  * @retval The last event
  */
static inline uint32_t i2c_get_last_event(I2C_TypeDef* I2Cx)
{
  uint32_t lastevent;
  uint32_t flag1, flag2;

  /* Read the I2Cx status register */
  flag1 = I2Cx->SR1;
  flag2 = I2Cx->SR2;
  flag2 = flag2 << 16;

  /* Get the last event value from I2C status register */
  lastevent = (flag1 | flag2) & FLAG_MASK;

  /* Return status */
  return lastevent;
}

/**
  * @brief  Checks whether the specified I2C flag is set or not.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg I2C_FLAG_DUALF: Dual flag (Slave mode)
  *            @arg I2C_FLAG_SMBHOST: SMBus host header (Slave mode)
  *            @arg I2C_FLAG_SMBDEFAULT: SMBus default header (Slave mode)
  *            @arg I2C_FLAG_GENCALL: General call header flag (Slave mode)
  *            @arg I2C_FLAG_TRA: Transmitter/Receiver flag
  *            @arg I2C_FLAG_BUSY: Bus busy flag
  *            @arg I2C_FLAG_MSL: Master/Slave flag
  *            @arg I2C_FLAG_SMBALERT: SMBus Alert flag
  *            @arg I2C_FLAG_TIMEOUT: Timeout or Tlow error flag
  *            @arg I2C_FLAG_PECERR: PEC error in reception flag
  *            @arg I2C_FLAG_OVR: Overrun/Underrun flag (Slave mode)
  *            @arg I2C_FLAG_AF: Acknowledge failure flag
  *            @arg I2C_FLAG_ARLO: Arbitration lost flag (Master mode)
  *            @arg I2C_FLAG_BERR: Bus error flag
  *            @arg I2C_FLAG_TXE: Data register empty flag (Transmitter)
  *            @arg I2C_FLAG_RXNE: Data register not empty (Receiver) flag
  *            @arg I2C_FLAG_STOPF: Stop detection flag (Slave mode)
  *            @arg I2C_FLAG_ADD10: 10-bit header sent flag (Master mode)
  *            @arg I2C_FLAG_BTF: Byte transfer finished flag
  *            @arg I2C_FLAG_ADDR: Address sent flag (Master mode) "ADSL"
  *                                Address matched flag (Slave mode)"ENDAD"
  *            @arg I2C_FLAG_SB: Start bit flag (Master mode)
  * @retval The new state of I2C_FLAG (SET or RESET).
  */
static inline bool i2c_get_flag_status(I2C_TypeDef* I2Cx, uint32_t I2C_FLAG)
{
  __IO uint32_t i2creg, i2cxbase;

  /* Get the I2Cx peripheral base address */
  i2cxbase = (uint32_t)I2Cx;

  /* Read flag register index */
  i2creg = I2C_FLAG >> 28;

  /* Get bit[23:0] of the flag */
  I2C_FLAG &= FLAG_MASK;

  if(i2creg != 0)
  {
    /* Get the I2Cx SR1 register address */
    i2cxbase += 0x14;
  }
  else
  {
    /* Flag in I2Cx SR2 Register */
    I2C_FLAG = (uint32_t)(I2C_FLAG >> 16);
    /* Get the I2Cx SR2 register address */
    i2cxbase += 0x18;
  }
  return (((*(__IO uint32_t *)i2cxbase) & I2C_FLAG) != (uint32_t)0);

}

/**
  * @brief  Clears the I2Cx's pending flags.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg I2C_FLAG_SMBALERT: SMBus Alert flag
  *            @arg I2C_FLAG_TIMEOUT: Timeout or Tlow error flag
  *            @arg I2C_FLAG_PECERR: PEC error in reception flag
  *            @arg I2C_FLAG_OVR: Overrun/Underrun flag (Slave mode)
  *            @arg I2C_FLAG_AF: Acknowledge failure flag
  *            @arg I2C_FLAG_ARLO: Arbitration lost flag (Master mode)
  *            @arg I2C_FLAG_BERR: Bus error flag
  *
  * @note   STOPF (STOP detection) is cleared by software sequence: a read operation
  *          to I2C_SR1 register (I2C_GetFlagStatus()) followed by a write operation
  *          to I2C_CR1 register (I2C_Cmd() to re-enable the I2C peripheral).
  * @note   ADD10 (10-bit header sent) is cleared by software sequence: a read
  *          operation to I2C_SR1 (I2C_GetFlagStatus()) followed by writing the
  *          second byte of the address in DR register.
  * @note   BTF (Byte Transfer Finished) is cleared by software sequence: a read
  *          operation to I2C_SR1 register (I2C_GetFlagStatus()) followed by a
  *          read/write to I2C_DR register (I2C_SendData()).
  * @note   ADDR (Address sent) is cleared by software sequence: a read operation to
  *          I2C_SR1 register (I2C_GetFlagStatus()) followed by a read operation to
  *          I2C_SR2 register ((void)(I2Cx->SR2)).
  * @note   SB (Start Bit) is cleared software sequence: a read operation to I2C_SR1
  *          register (I2C_GetFlagStatus()) followed by a write operation to I2C_DR
  *          register (I2C_SendData()).
  *
  * @retval None
  */
static inline void i2c_clear_flag(I2C_TypeDef* I2Cx, uint32_t I2C_FLAG)
{
  uint32_t flagpos;
  /* Get the I2C flag position */
  flagpos = I2C_FLAG & FLAG_MASK;
  /* Clear the selected I2C flag */
  I2Cx->SR1 = (uint16_t)~flagpos;
}

/**
  * @brief  Checks whether the specified I2C interrupt has occurred or not.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_IT: specifies the interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg I2C_IT_SMBALERT: SMBus Alert flag
  *            @arg I2C_IT_TIMEOUT: Timeout or Tlow error flag
  *            @arg I2C_IT_PECERR: PEC error in reception flag
  *            @arg I2C_IT_OVR: Overrun/Underrun flag (Slave mode)
  *            @arg I2C_IT_AF: Acknowledge failure flag
  *            @arg I2C_IT_ARLO: Arbitration lost flag (Master mode)
  *            @arg I2C_IT_BERR: Bus error flag
  *            @arg I2C_IT_TXE: Data register empty flag (Transmitter)
  *            @arg I2C_IT_RXNE: Data register not empty (Receiver) flag
  *            @arg I2C_IT_STOPF: Stop detection flag (Slave mode)
  *            @arg I2C_IT_ADD10: 10-bit header sent flag (Master mode)
  *            @arg I2C_IT_BTF: Byte transfer finished flag
  *            @arg I2C_IT_ADDR: Address sent flag (Master mode) "ADSL"
  *                              Address matched flag (Slave mode)"ENDAD"
  *            @arg I2C_IT_SB: Start bit flag (Master mode)
  * @retval The new state of I2C_IT (SET or RESET).
  */
static inline bool i2c_get_it_status(I2C_TypeDef* I2Cx, uint32_t I2C_IT)
{
  uint32_t enablestatus;

  /* Check if the interrupt source is enabled or not */
  enablestatus = (uint32_t)(((I2C_IT & ITEN_MASK) >> 16) & (I2Cx->CR2)) ;

  /* Get bit[23:0] of the flag */
  I2C_IT &= FLAG_MASK;

  /* Check the status of the specified I2C flag */
  return (((I2Cx->SR1 & I2C_IT) != (uint32_t)0) && enablestatus);
}


/**
  * @brief  Clears the I2Cx's interrupt pending bits.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_IT: specifies the interrupt pending bit to clear.
  *          This parameter can be any combination of the following values:
  *            @arg I2C_IT_SMBALERT: SMBus Alert interrupt
  *            @arg I2C_IT_TIMEOUT: Timeout or Tlow error interrupt
  *            @arg I2C_IT_PECERR: PEC error in reception  interrupt
  *            @arg I2C_IT_OVR: Overrun/Underrun interrupt (Slave mode)
  *            @arg I2C_IT_AF: Acknowledge failure interrupt
  *            @arg I2C_IT_ARLO: Arbitration lost interrupt (Master mode)
  *            @arg I2C_IT_BERR: Bus error interrupt
  *
  * @note   STOPF (STOP detection) is cleared by software sequence: a read operation
  *          to I2C_SR1 register (I2C_GetITStatus()) followed by a write operation to
  *          I2C_CR1 register (I2C_Cmd() to re-enable the I2C peripheral).
  * @note   ADD10 (10-bit header sent) is cleared by software sequence: a read
  *          operation to I2C_SR1 (I2C_GetITStatus()) followed by writing the second
  *          byte of the address in I2C_DR register.
  * @note   BTF (Byte Transfer Finished) is cleared by software sequence: a read
  *          operation to I2C_SR1 register (I2C_GetITStatus()) followed by a
  *          read/write to I2C_DR register (I2C_SendData()).
  * @note   ADDR (Address sent) is cleared by software sequence: a read operation to
  *          I2C_SR1 register (I2C_GetITStatus()) followed by a read operation to
  *          I2C_SR2 register ((void)(I2Cx->SR2)).
  * @note   SB (Start Bit) is cleared by software sequence: a read operation to
  *          I2C_SR1 register (I2C_GetITStatus()) followed by a write operation to
  *          I2C_DR register (I2C_SendData()).
  * @retval None
  */
static inline void i2c_clear_it_pending_bit(I2C_TypeDef* I2Cx, uint32_t I2C_IT)
{
  uint32_t flagpos;

  /* Get the I2C flag position */
  flagpos = I2C_IT & FLAG_MASK;

  /* Clear the selected I2C flag */
  I2Cx->SR1 = (uint16_t)~flagpos;
}

#ifdef __cplusplus
}
#endif

#undef CR1_CLEAR_MASK
#undef FLAG_MASK
#undef ITEN_MASK


