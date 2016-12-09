/*
 * =====================================================================================
 *
 *       Filename:  stm32crc.h
 *
 *    Description:  STM32CRC for all stm32 devs
 *
 *        Version:  1.0
 *        Created:  21.10.2016 21:52:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "stm32lib.h"

#ifndef STM32_CRC_HARDWARE_VERSION
#error STM32_CRC_HARDWARE_VERSION not defined
#endif


#ifdef __cplusplus
namespace stm32 {
#endif



#if STM32_CRC_HARDWARE_VERSION == 2

/**
  * @brief  Deinitializes CRC peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
static inline void crc_deinit(void)
{
  /* Set DR register to reset value */
  CRC->DR = 0xFFFFFFFF;

  /* Set the POL register to the reset value: 0x04C11DB7 */
  CRC->POL = 0x04C11DB7;

  /* Reset IDR register */
  CRC->IDR = 0x00;

  /* Set INIT register to reset value */
  CRC->INIT = 0xFFFFFFFF;

  /* Reset the CRC calculation unit */
  CRC->CR = CRC_CR_RESET;
}
#endif


/**
  * @brief  Resets the CRC calculation unit and sets INIT register content in DR register.
  * @param  None
  * @retval None
  */
static inline void crc_reset_dr(void)
{
  /* Reset CRC generator */
  CRC->CR |= CRC_CR_RESET;
}


#if STM32_CRC_HARDWARE_VERSION == 2
/**
  * @brief  Selects the polynomial size.
  * @param  CRC_PolSize: Specifies the polynomial size.
  *          This parameter can be:
  *           @arg CRC_PolSize_7: 7-bit polynomial for CRC calculation
  *           @arg CRC_PolSize_8: 8-bit polynomial for CRC calculation
  *           @arg CRC_PolSize_16: 16-bit polynomial for CRC calculation
  *           @arg CRC_PolSize_32: 32-bit polynomial for CRC calculation
  * @retval None
  */
static inline void crc_polynomial_size_select(uint32_t CRC_PolSize)
{
  uint32_t tmpcr = 0;

  /* Get CR register value */
  tmpcr = CRC->CR;

  /* Reset POL_SIZE bits */
  tmpcr &= (uint32_t)~((uint32_t)CRC_CR_POLSIZE);
  /* Set the polynomial size */
  tmpcr |= (uint32_t)CRC_PolSize;

  /* Write to CR register */
  CRC->CR = (uint32_t)tmpcr;
}


/**
  * @brief  Selects the reverse operation to be performed on input data.
  * @param  CRC_ReverseInputData: Specifies the reverse operation on input data.
  *          This parameter can be:
  *            @arg CRC_ReverseInputData_No: No reverse operation is performed
  *            @arg CRC_ReverseInputData_8bits: reverse operation performed on 8 bits
  *            @arg CRC_ReverseInputData_16bits: reverse operation performed on 16 bits
  *            @arg CRC_ReverseInputData_32bits: reverse operation performed on 32 bits
  * @retval None
  */
static inline void crc_reverse_input_data_select(uint32_t CRC_ReverseInputData)
{
  uint32_t tmpcr = 0;

  /* Get CR register value */
  tmpcr = CRC->CR;

  /* Reset REV_IN bits */
  tmpcr &= (uint32_t)~((uint32_t)CRC_CR_REV_IN);
  /* Set the reverse operation */
  tmpcr |= (uint32_t)CRC_ReverseInputData;

  /* Write to CR register */
  CRC->CR = (uint32_t)tmpcr;
}

/**
  * @brief  Enables or disable the reverse operation on output data.
  *         The reverse operation on output data is performed on 32-bit.
  * @param  NewState: new state of the reverse operation on output data.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void crc_reverse_output_data_cmd(bool enable)
{
  if (enable)
  {
    /* Enable reverse operation on output data */
    CRC->CR |= CRC_CR_REV_OUT;
  }
  else
  {
    /* Disable reverse operation on output data */
    CRC->CR &= (uint32_t)~((uint32_t)CRC_CR_REV_OUT);
  }
}

/**
  * @brief  Initializes the INIT register.
  * @note   After resetting CRC calculation unit, CRC_InitValue is stored in DR register
  * @param  CRC_InitValue: Programmable initial CRC value
  * @retval None
  */
static inline void crc_set_init_register(uint32_t CRC_InitValue)
{
  CRC->INIT = CRC_InitValue;
}

/**
  * @brief  Initializes the polynomail coefficients.
  * @param  CRC_Pol: Polynomial to be used for CRC calculation.
  * @retval None
  */
static inline void crc_set_polynomial(uint32_t CRC_Pol)
{
  CRC->POL = CRC_Pol;
}

#endif

/**
  * @brief  Computes the 32-bit CRC of a given data word(32-bit).
  * @param  CRC_Data: data word(32-bit) to compute its CRC
  * @retval 32-bit CRC
  */
static inline uint32_t crc_calc_crc(uint32_t CRC_Data)
{
  CRC->DR = CRC_Data;
  return (CRC->DR);
}


#if STM32_CRC_HARDWARE_VERSION == 2

/**
  * @brief  Computes the 16-bit CRC of a given 16-bit data.
  * @param  CRC_Data: data half-word(16-bit) to compute its CRC
  * @retval 16-bit CRC
  */
static inline uint32_t crc_calc_crc_16bits(uint16_t CRC_Data)
{
  *(uint16_t*)(CRC_BASE) = (uint16_t) CRC_Data;
  
  return (CRC->DR);
}

/**
  * @brief  Computes the 8-bit CRC of a given 8-bit data.
  * @param  CRC_Data: 8-bit data to compute its CRC
  * @retval 8-bit CRC
  */
static inline uint32_t crc_calc_crc_8bits(uint8_t CRC_Data)
{
  *(uint8_t*)(CRC_BASE) = (uint8_t) CRC_Data;

  return (CRC->DR);
}

#endif



/**
  * @brief  Computes the 32-bit CRC of a given buffer of data word(32-bit).
  * @param  pBuffer: pointer to the buffer containing the data to be computed
  * @param  BufferLength: length of the buffer to be computed
  * @retval 32-bit CRC
  */
static inline uint32_t crc_calc_block_crc(const uint32_t pBuffer[], uint32_t BufferLength)
{
  uint32_t index = 0;
  
  for(index = 0; index < BufferLength; index++)
  {
    CRC->DR = pBuffer[index];
  }
  return (CRC->DR);
}

/**
  * @brief  Returns the current CRC value.
  * @param  None
  * @retval 32-bit CRC
  */
static inline uint32_t crc_get_crc(void)
{
  return (CRC->DR);
}


/**
  * @brief  Stores an 8-bit data in the Independent Data(ID) register.
  * @param  CRC_IDValue: 8-bit value to be stored in the ID register
  * @retval None
  */
static inline void crc_set_id_register(uint8_t CRC_IDValue)
{
  CRC->IDR = CRC_IDValue;
}

/**
  * @brief  Returns the 8-bit data stored in the Independent Data(ID) register
  * @param  None
  * @retval 8-bit value of the ID register
  */
static inline uint8_t crc_get_id_register(void)
{
  return (CRC->IDR);
}

#ifdef __cplusplus
}
#endif


