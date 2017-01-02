
/*
 * stm32spi.h
 *
 *	STM32SPI common  for F1 F2 and F4 platform
 *  Created on: 27-11-2012
 *      Author: lucck
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "stm32fxxx_spi.h"
#include <stm32lib.h>
#include <stm32rcc.h>

#define CR1_CLEAR_MASK            ((uint16_t)0x3040)
#define I2SCFGR_CLEAR_MASK        ((uint16_t)0xF040)
#define SPI_CR2_FRF               ((uint16_t)0x0010)
#ifdef STM32MCU_MAJOR_TYPE_F1
#define I2S2_CLOCK_SRC       ((uint32_t)(0x00020000))
#define I2S3_CLOCK_SRC       ((uint32_t)(0x00040000))
#define I2S_MUL_MASK         ((uint32_t)(0x0000F000))
#define I2S_DIV_MASK         ((uint32_t)(0x000000F0))
#endif
#define CR1_CLEAR_MASK2      ((uint16_t)0xFFFB)
#define CR2_LDMA_MASK        ((uint16_t)0x9FFF)
#ifdef __cplusplus
namespace stm32 {
#endif

/**
  * @brief  Deinitialize the SPIx peripheral registers to their default reset values.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode.
  *
  * @note   The extended I2S blocks (ie. I2S2ext and I2S3ext blocks) are deinitialized
  *         when the relative I2S peripheral is deinitialized (the extended block's clock
  *         is managed by the I2S peripheral clock).
  *
  * @retval None
  */
static inline void spi_i2s_deinit(SPI_TypeDef* SPIx)
{
  if (SPIx == SPI1)
  {
    /* Enable SPI1 reset state */
    rcc_apb2_periph_reset_cmd(RCC_APB2Periph_SPI1, true );
    /* Release SPI1 from reset state */
    rcc_apb2_periph_reset_cmd(RCC_APB2Periph_SPI1, false );
  }
  else if (SPIx == SPI2)
  {
     /* Enable SPI2 reset state */
	 rcc_apb1_periph_reset_cmd(RCC_APB1Periph_SPI2, true );
     /* Release SPI2 from reset state */
	 rcc_apb1_periph_reset_cmd(RCC_APB1Periph_SPI2, false );
  }
 #ifdef SPI3
  else if (SPIx == SPI3)
  {
      /* Enable SPI3 reset state */
      rcc_apb1_periph_reset_cmd(RCC_APB1Periph_SPI3, true );
      /* Release SPI3 from reset state */
      rcc_apb1_periph_reset_cmd(RCC_APB1Periph_SPI3, false );
  }
 #endif
 #ifdef SPI4
  else if (SPIx == SPI4)
  {
     rcc_apb2_periph_reset_cmd(RCC_APB2Periph_SPI4, true);
     /* Release SPI4 from reset state */
	 rcc_apb2_periph_reset_cmd(RCC_APB2Periph_SPI4, false);
  }
 #endif
}

/**
  * @brief  Initializes the SPIx peripheral according to the specified
  *         parameters in the SPI_InitStruct.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure that
  *         contains the configuration information for the specified SPI peripheral.
  * @retval None
  */
static inline void spi_init(SPI_TypeDef* SPIx, uint16_t direction, uint16_t mode ,
		uint16_t data_size, uint16_t cpol, uint16_t cpha, uint16_t nss,
		uint16_t baudrate_prescaler, uint16_t first_bit, int crc_polynomial )
{
	uint16_t tmpreg = 0;
#ifdef STM32_SPI_V2
	/* Get the SPIx CR1 value */
	tmpreg = SPIx->CR1;
	/* Clear BIDIMode, BIDIOE, RxONLY, SSM, SSI, LSBFirst, BR, CPOL and CPHA bits */
	tmpreg &= CR1_CLEAR_MASK;
	/* Configure SPIx: direction, NSS management, first transmitted bit, BaudRate prescaler
	   master/slave mode, CPOL and CPHA */
	/* Set BIDImode, BIDIOE and RxONLY bits according to SPI_Direction value */
	/* Set SSM, SSI bit according to SPI_NSS values */
	/* Set LSBFirst bit according to SPI_FirstBit value */
	/* Set BR bits according to SPI_BaudRatePrescaler value */
	/* Set CPOL bit according to SPI_CPOL value */
	/* Set CPHA bit according to SPI_CPHA value */
	tmpreg |= (uint16_t)((uint32_t)direction | first_bit |
			cpol | cpha |
			nss | baudrate_prescaler ); 
	/* Write to SPIx CR1 */
	SPIx->CR1 = tmpreg;
	/* Get the SPIx CR2 value */
	tmpreg = SPIx->CR2;
	/* Clear DS[3:0] bits */
	tmpreg &=(uint16_t)~SPI_CR2_DS;
	/* Configure SPIx: Data Size */
	tmpreg |= (uint16_t)(data_size);
	/* Write to SPIx CR2 */
	SPIx->CR2 = tmpreg;
	/* Write to SPIx CRCPOLY */
	SPIx->CRCPR = crc_polynomial;
		/* Get the SPIx CR1 value */
	tmpreg = SPIx->CR1;
	/* Clear MSTR bit */
	tmpreg &= CR1_CLEAR_MASK2;
	/* Configure SPIx: master/slave mode */  
	/* Set MSTR bit according to SPI_Mode */
	tmpreg |= (uint16_t)((uint32_t)mode); 
	/* Write to SPIx CR1 */
	SPIx->CR1 = tmpreg; 
	/* Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register) */
	SPIx->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SMOD);
#else
	/* Get the SPIx CR1 value */
	tmpreg = SPIx->CR1;
	/* Clear BIDIMode, BIDIOE, RxONLY, SSM, SSI, LSBFirst, BR, MSTR, CPOL and CPHA bits */
	tmpreg &= CR1_CLEAR_MASK;
	/* Configure SPIx: direction, NSS management, first transmitted bit, BaudRate prescaler
	   master/salve mode, CPOL and CPHA */
	/* Set BIDImode, BIDIOE and RxONLY bits according to SPI_Direction value */
	/* Set SSM, SSI and MSTR bits according to SPI_Mode and SPI_NSS values */
	/* Set LSBFirst bit according to SPI_FirstBit value */
	/* Set BR bits according to SPI_BaudRatePrescaler value */
	/* Set CPOL bit according to SPI_CPOL value */
	/* Set CPHA bit according to SPI_CPHA value */
	tmpreg |= (uint16_t)((uint32_t)direction| mode |
			data_size | cpol |cpha  | nss | baudrate_prescaler | first_bit);
	/* Write to SPIx CR1 */
	SPIx->CR1 = tmpreg;

	/* Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register) */
	SPIx->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SMOD);
	/* Write to SPIx CRCPOLY */
	if(crc_polynomial > 0)
		SPIx->CRCPR = crc_polynomial;
#endif
}

#if defined(STM32MCU_MAJOR_TYPE_F2) || defined(STM32MCU_MAJOR_TYPE_F4)
/**
  * @brief  Initializes the SPIx peripheral according to the specified
  *         parameters in the I2S_InitStruct.
  * @param  SPIx: where x can be  2 or 3 to select the SPI peripheral (configured in I2S mode).
  * @param  I2S_InitStruct: pointer to an I2S_InitTypeDef structure that
  *         contains the configuration information for the specified SPI peripheral
  *         configured in I2S mode.
  *
  * @note   The function calculates the optimal prescaler needed to obtain the most
  *         accurate audio frequency (depending on the I2S clock source, the PLL values
  *         and the product configuration). But in case the prescaler value is greater
  *         than 511, the default value (0x02) will be configured instead.
  *
  * @note   if an external clock is used as source clock for the I2S, then the define
  *         I2S_EXTERNAL_CLOCK_VAL in file stm32f4xx_conf.h should be enabled and set
  *         to the value of the the source clock frequency (in Hz).
  *
  * @retval None
  */
static inline void i2s_init( SPI_TypeDef* SPIx, uint16_t mode, uint16_t standard, uint16_t data_format,
		uint16_t mclk_output, uint32_t audio_freq, uint16_t cpol )
{
  uint16_t tmpreg = 0, i2sdiv = 2, i2sodd = 0, packetlength = 1;
  uint32_t tmp = 0;
  uint32_t i2sclk = 0;
#ifndef I2S_EXTERNAL_CLOCK_VAL
  uint32_t pllm = 0, plln = 0, pllr = 0;
#endif /* I2S_EXTERNAL_CLOCK_VAL */
  /* Clear I2SMOD, I2SE, I2SCFG, PCMSYNC, I2SSTD, CKPOL, DATLEN and CHLEN bits */
  SPIx->I2SCFGR &= I2SCFGR_CLEAR_MASK;
  SPIx->I2SPR = 0x0002;

  /* Get the I2SCFGR register value */
  tmpreg = SPIx->I2SCFGR;

  /* If the default value has to be written, reinitialize i2sdiv and i2sodd*/
  if(audio_freq == I2S_AudioFreq_Default)
  {
    i2sodd = (uint16_t)0;
    i2sdiv = (uint16_t)2;
  }
  /* If the requested audio frequency is not the default, compute the prescaler */
  else
  {
    /* Check the frame length (For the Prescaler computing) *******************/
    if(data_format == I2S_DataFormat_16b)
    {
      /* Packet length is 16 bits */
      packetlength = 1;
    }
    else
    {
      /* Packet length is 32 bits */
      packetlength = 2;
    }

    /* Get I2S source Clock frequency  ****************************************/

    /* If an external I2S clock has to be used, this define should be set
       in the project configuration or in the stm32f4xx_conf.h file */
  #ifdef I2S_EXTERNAL_CLOCK_VAL
    /* Set external clock as I2S clock source */
    if ((RCC->CFGR & RCC_CFGR_I2SSRC) == 0)
    {
      RCC->CFGR |= (uint32_t)RCC_CFGR_I2SSRC;
    }

    /* Set the I2S clock to the external clock  value */
    i2sclk = I2S_EXTERNAL_CLOCK_VAL;

  #else /* There is no define for External I2S clock source */
    /* Set PLLI2S as I2S clock source */
    if ((RCC->CFGR & RCC_CFGR_I2SSRC) != 0)
    {
      RCC->CFGR &= ~(uint32_t)RCC_CFGR_I2SSRC;
    }

    /* Get the PLLI2SN value */
    plln = (uint32_t)(((RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SN) >> 6) & \
                      (RCC_PLLI2SCFGR_PLLI2SN >> 6));

    /* Get the PLLI2SR value */
    pllr = (uint32_t)(((RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SR) >> 28) & \
                      (RCC_PLLI2SCFGR_PLLI2SR >> 28));

    /* Get the PLLM value */
    pllm = (uint32_t)(RCC->PLLCFGR & RCC_PLLCFGR_PLLM);

    /* Get the I2S source clock value */
    i2sclk = (uint32_t)(((HSE_VALUE / pllm) * plln) / pllr);
  #endif /* I2S_EXTERNAL_CLOCK_VAL */

    /* Compute the Real divider depending on the MCLK output state, with a floating point */
    if(mclk_output == I2S_MCLKOutput_Enable)
    {
      /* MCLK output is enabled */
      tmp = (uint16_t)(((((i2sclk / 256) * 10) / audio_freq)) + 5);
    }
    else
    {
      /* MCLK output is disabled */
      tmp = (uint16_t)(((((i2sclk / (32 * packetlength)) *10 ) / audio_freq)) + 5);
    }

    /* Remove the flatting point */
    tmp = tmp / 10;

    /* Check the parity of the divider */
    i2sodd = (uint16_t)(tmp & (uint16_t)0x0001);

    /* Compute the i2sdiv prescaler */
    i2sdiv = (uint16_t)((tmp - i2sodd) / 2);

    /* Get the Mask for the Odd bit (SPI_I2SPR[8]) register */
    i2sodd = (uint16_t) (i2sodd << 8);
  }

  /* Test if the divider is 1 or 0 or greater than 0xFF */
  if ((i2sdiv < 2) || (i2sdiv > 0xFF))
  {
    /* Set the default values */
    i2sdiv = 2;
    i2sodd = 0;
  }

  /* Write to SPIx I2SPR register the computed value */
  SPIx->I2SPR = (uint16_t)((uint16_t)i2sdiv | (uint16_t)(i2sodd | (uint16_t)mclk_output));

  /* Configure the I2S with the SPI_InitStruct values */
  tmpreg |= (uint16_t)((uint16_t)SPI_I2SCFGR_I2SMOD | (uint16_t)(mode | (uint16_t)(standard | (uint16_t)(data_format | (uint16_t)cpol))));

  /* Write to SPIx I2SCFGR */
  SPIx->I2SCFGR = tmpreg;
}
#elif defined(STM32MCU_MAJOR_TYPE_F1) || defined(STM32MCU_MAJOR_TYPE_F3 ) || \
	defined(STM32MCU_MAJOR_TYPE_F37) 

static inline void i2s_init(SPI_TypeDef* SPIx, uint16_t mode, uint16_t standard, uint16_t data_format,
		uint16_t mclk_output, uint32_t audio_freq, uint16_t cpol, uint32_t sysclk_freq)
{
  uint16_t tmpreg = 0, i2sdiv = 2, i2sodd = 0, packetlength = 1;
  uint32_t tmp = 0;
  //RCC_ClocksTypeDef RCC_Clocks;
  uint32_t sourceclock = 0;

  /* Clear I2SMOD, I2SE, I2SCFG, PCMSYNC, I2SSTD, CKPOL, DATLEN and CHLEN bits */
  SPIx->I2SCFGR &= I2SCFGR_CLEAR_MASK;
  SPIx->I2SPR = 0x0002;

  /* Get the I2SCFGR register value */
  tmpreg = SPIx->I2SCFGR;

  /* If the default value has to be written, reinitialize i2sdiv and i2sodd*/
  if(audio_freq == I2S_AudioFreq_Default)
  {
    i2sodd = (uint16_t)0;
    i2sdiv = (uint16_t)2;
  }
  /* If the requested audio frequency is not the default, compute the prescaler */
  else
  {
    /* Check the frame length (For the Prescaler computing) */
    if(data_format == I2S_DataFormat_16b)
    {
      /* Packet length is 16 bits */
      packetlength = 1;
    }
    else
    {
      /* Packet length is 32 bits */
      packetlength = 2;
    }

#ifdef STM32F10X_CL
    /* Get the I2S clock source mask depending on the peripheral number */
    if(((uint32_t)SPIx) == SPI2_BASE)
    {
      /* The mask is relative to I2S2 */
      tmp = I2S2_CLOCK_SRC;
    }
    else
    {
      /* The mask is relative to I2S3 */
      tmp = I2S3_CLOCK_SRC;
    }

    /* Check the I2S clock source configuration depending on the Device:
       Only Connectivity line devices have the PLL3 VCO clock */
    if((RCC->CFGR2 & tmp) != 0)
    {
      /* Get the configuration bits of RCC PLL3 multiplier */
      tmp = (uint32_t)((RCC->CFGR2 & I2S_MUL_MASK) >> 12);

      /* Get the value of the PLL3 multiplier */
      if((tmp > 5) && (tmp < 15))
      {
        /* Multiplier is between 8 and 14 (value 15 is forbidden) */
        tmp += 2;
      }
      else
      {
        if (tmp == 15)
        {
          /* Multiplier is 20 */
          tmp = 20;
        }
      }
      /* Get the PREDIV2 value */
      sourceclock = (uint32_t)(((RCC->CFGR2 & I2S_DIV_MASK) >> 4) + 1);

      /* Calculate the Source Clock frequency based on PLL3 and PREDIV2 values */
      sourceclock = (uint32_t) ((HSE_Value / sourceclock) * tmp * 2);
    }
    else
    {

      /* Get the source clock value: based on System Clock value */
      sourceclock = sysclk_freq;
    }
#else /* STM32F10X_HD */
    /* Get the source clock value: based on System Clock value */
    sourceclock = sysclk_freq;
#endif /* STM32F10X_CL */

    /* Compute the Real divider depending on the MCLK output state with a floating point */
    if(mclk_output == I2S_MCLKOutput_Enable)
    {
      /* MCLK output is enabled */
      tmp = (uint16_t)(((((sourceclock / 256) * 10) / audio_freq)) + 5);
    }
    else
    {
      /* MCLK output is disabled */
      tmp = (uint16_t)(((((sourceclock / (32 * packetlength)) *10 ) / audio_freq)) + 5);
    }

    /* Remove the floating point */
    tmp = tmp / 10;

    /* Check the parity of the divider */
    i2sodd = (uint16_t)(tmp & (uint16_t)0x0001);

    /* Compute the i2sdiv prescaler */
    i2sdiv = (uint16_t)((tmp - i2sodd) / 2);

    /* Get the Mask for the Odd bit (SPI_I2SPR[8]) register */
    i2sodd = (uint16_t) (i2sodd << 8);
  }

  /* Test if the divider is 1 or 0 or greater than 0xFF */
  if ((i2sdiv < 2) || (i2sdiv > 0xFF))
  {
    /* Set the default values */
    i2sdiv = 2;
    i2sodd = 0;
  }

  /* Write to SPIx I2SPR register the computed value */
  SPIx->I2SPR = (uint16_t)(i2sdiv | (uint16_t)(i2sodd | (uint16_t)mclk_output));

  /* Configure the I2S with the SPI_InitStruct values */
  tmpreg |= (uint16_t)(SPI_I2SCFGR_I2SMOD | (uint16_t)(mode | (uint16_t)(standard | (uint16_t)(data_format | (uint16_t)cpol))));

  /* Write to SPIx I2SCFGR */
  SPIx->I2SCFGR = tmpreg;
}
#endif

/**
  * @brief  Enables or disables the specified SPI peripheral.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  NewState: new state of the SPIx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void spi_cmd(SPI_TypeDef* SPIx, bool enable)
{
  if ( enable )
  {
    /* Enable the selected SPI peripheral */
    SPIx->CR1 |= SPI_CR1_SPE;
  }
  else
  {
    /* Disable the selected SPI peripheral */
    SPIx->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_SPE);
  }
}

/** Check if SPI is enabled */
static inline bool spi_is_enabled( SPI_TypeDef* SPIx )
{
	return !!(SPIx->CR1 & SPI_CR1_SPE);
}


/**
  * @brief  Enables or disables the specified SPI peripheral (in I2S mode).
  * @param  SPIx: where x can be 2 or 3 to select the SPI peripheral (or I2Sxext
  *         for full duplex mode).
  * @param  NewState: new state of the SPIx peripheral.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void i2s_cmd(SPI_TypeDef* SPIx, bool enable)
{
  if ( enable )
  {
    /* Enable the selected SPI peripheral (in I2S mode) */
    SPIx->I2SCFGR |= SPI_I2SCFGR_I2SE;
  }
  else
  {
    /* Disable the selected SPI peripheral in I2S mode */
    SPIx->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SE);
  }
}

/**
  * @brief  Configures the data size for the selected SPI.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_DataSize: specifies the SPI data size.
  *          This parameter can be one of the following values:
  *            @arg SPI_DataSize_16b: Set data frame format to 16bit
  *            @arg SPI_DataSize_8b: Set data frame format to 8bit
  * @retval None
  */
static inline void spi_data_size_config(SPI_TypeDef* SPIx, uint16_t SPI_DataSize)
{
#ifdef STM32_SPI_V2
  /* Read the CR2 register */
  uint16_t tmpreg = SPIx->CR2;
  /* Clear DS[3:0] bits */
  tmpreg &= (uint16_t)~SPI_CR2_DS;
  /* Set new DS[3:0] bits value */
  tmpreg |= SPI_DataSize;
  SPIx->CR2 = tmpreg;
#else
  /* Clear DFF bit */
  SPIx->CR1 &= (uint16_t)~SPI_DataSize_16b;
  /* Set new DFF bit value */
  SPIx->CR1 |= SPI_DataSize;
#endif
}

/**
  * @brief  Selects the data transfer direction in bidirectional mode for the specified SPI.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_Direction: specifies the data transfer direction in bidirectional mode.
  *          This parameter can be one of the following values:
  *            @arg SPI_Direction_Tx: Selects Tx transmission direction
  *            @arg SPI_Direction_Rx: Selects Rx receive direction
  * @retval None
  */
static inline void spi_bidirectional_line_config(SPI_TypeDef* SPIx, uint16_t SPI_Direction)
{
  if (SPI_Direction == SPI_Direction_Tx)
  {
    /* Set the Tx only mode */
    SPIx->CR1 |= SPI_Direction_Tx;
  }
  else
  {
    /* Set the Rx only mode */
    SPIx->CR1 &= SPI_Direction_Rx;
  }
}

/**
  * @brief  Configures internally by software the NSS pin for the selected SPI.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_NSSInternalSoft: specifies the SPI NSS internal state.
  *          This parameter can be one of the following values:
  *            @arg SPI_NSSInternalSoft_Set: Set NSS pin internally
  *            @arg SPI_NSSInternalSoft_Reset: Reset NSS pin internally
  * @retval None
  */
static inline void spi_nss_internal_software_config(SPI_TypeDef* SPIx, uint16_t SPI_NSSInternalSoft)
{
  if (SPI_NSSInternalSoft != SPI_NSSInternalSoft_Reset)
  {
    /* Set NSS pin internally by software */
    SPIx->CR1 |= SPI_NSSInternalSoft_Set;
  }
  else
  {
    /* Reset NSS pin internally by software */
    SPIx->CR1 &= SPI_NSSInternalSoft_Reset;
  }
}

/**
  * @brief  Enables or disables the SS output for the selected SPI.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  NewState: new state of the SPIx SS output.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void spi_ss_output_cmd(SPI_TypeDef* SPIx, bool enable)
{
  if ( enable )
  {
    /* Enable the selected SPI SS output */
    SPIx->CR2 |= (uint16_t)SPI_CR2_SSOE;
  }
  else
  {
    /* Disable the selected SPI SS output */
    SPIx->CR2 &= (uint16_t)~((uint16_t)SPI_CR2_SSOE);
  }
}

/**
  * @brief  Enables or disables the NSS pulse management mode.
  * @note   This function can be called only after the SPI_Init() function has 
  *         been called. 
  * @note   When TI mode is selected, the control bits NSSP is not taken into 
  *         consideration and are configured by hardware respectively to the 
  *         TI mode requirements. 
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  NewState: new state of the NSS pulse management mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
#ifdef SPI_CR2_NSSP
static inline void spi_nss_pulse_mode_cmd(SPI_TypeDef* SPIx, bool en )
{
  if ( en )
  {
    /* Enable the NSS pulse management mode */
    SPIx->CR2 |= SPI_CR2_NSSP;
  }
  else
  {
    /* Disable the NSS pulse management mode */
    SPIx->CR2 &= (uint16_t)~((uint16_t)SPI_CR2_NSSP);
  }
}
#endif



#ifdef SPI_CR2_FRF
/**
  * @brief  Enables or disables the SPIx/I2Sx DMA interface.
  *
  * @note   This function can be called only after the SPI_Init() function has
  *         been called.
  * @note   When TI mode is selected, the control bits SSM, SSI, CPOL and CPHA
  *         are not taken into consideration and are configured by hardware
  *         respectively to the TI mode requirements.
  *
  * @param  SPIx: where x can be 1, 2 or 3
  * @param  NewState: new state of the selected SPI TI communication mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void spi_ti_mode_cmd(SPI_TypeDef* SPIx, bool enable)
{

  if ( enable )
  {
    /* Enable the TI mode for the selected SPI peripheral */
    SPIx->CR2 |= SPI_CR2_FRF;
  }
  else
  {
    /* Disable the TI mode for the selected SPI peripheral */
    SPIx->CR2 &= (uint16_t)~SPI_CR2_FRF;
  }
}
#endif

/**
  * @brief  Configures the full duplex mode for the I2Sx peripheral using its
  *         extension I2Sxext according to the specified parameters in the
  *         I2S_InitStruct.
  * @param  I2Sxext: where x can be  2 or 3 to select the I2S peripheral extension block.
  * @param  I2S_InitStruct: pointer to an I2S_InitTypeDef structure that
  *         contains the configuration information for the specified I2S peripheral
  *         extension.
  *
  * @note   The structure pointed by I2S_InitStruct parameter should be the same
  *         used for the master I2S peripheral. In this case, if the master is
  *         configured as transmitter, the slave will be receiver and vice versa.
  *         Or you can force a different mode by modifying the field I2S_Mode to the
  *         value I2S_SlaveRx or I2S_SlaveTx indepedently of the master configuration.
  *
  * @note   The I2S full duplex extension can be configured in slave mode only.
  *
  * @retval None
  */
static inline void i2s_full_duplex_config(SPI_TypeDef* I2Sxext, uint16_t mode,
		uint16_t standard, uint16_t data_format, uint16_t cpol)
{
   uint16_t tmpreg = 0, tmp = 0;

  /* Clear I2SMOD, I2SE, I2SCFG, PCMSYNC, I2SSTD, CKPOL, DATLEN and CHLEN bits */
  I2Sxext->I2SCFGR &= I2SCFGR_CLEAR_MASK;
  I2Sxext->I2SPR = 0x0002;

  /* Get the I2SCFGR register value */
  tmpreg = I2Sxext->I2SCFGR;

  /* Get the mode to be configured for the extended I2S */
  if ((mode == I2S_Mode_MasterTx) || (mode == I2S_Mode_SlaveTx))
  {
    tmp = I2S_Mode_SlaveRx;
  }
  else
  {
    if ((mode == I2S_Mode_MasterRx) || (mode == I2S_Mode_SlaveRx))
    {
      tmp = I2S_Mode_SlaveTx;
    }
  }

  /* Configure the I2S with the SPI_InitStruct values */
  tmpreg |= (uint16_t)((uint16_t)SPI_I2SCFGR_I2SMOD | (uint16_t)(tmp | (uint16_t)(standard | (uint16_t)(data_format | (uint16_t)cpol))));

  /* Write to SPIx I2SCFGR */
  I2Sxext->I2SCFGR = tmpreg;
}

/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @retval The value of the received data.
  */
static inline uint16_t spi_i2s_receive_data(SPI_TypeDef* SPIx)
{
  /* Return the data in the DR register */
  return SPIx->DR;
}
static inline uint16_t spi_i2s_receive_data16(SPI_TypeDef* SPIx)
{
  /* Return the data in the DR register */
  return SPIx->DR;
}

/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral. 
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @retval The value of the received data.
  */
static inline uint8_t spi_receive_data8(SPI_TypeDef* SPIx)
{
  uint32_t spixbase;
  spixbase = (uint32_t)SPIx; 
  spixbase += 0x0C;
  return *(__IO uint8_t *) spixbase;
}


/**
  * @brief  Transmits a Data through the SPIx/I2Sx peripheral.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  Data: Data to be transmitted.
  * @retval None
  */
static inline void spi_i2s_send_data(SPI_TypeDef* SPIx, uint16_t Data)
{
  /* Write in the DR register the data to be sent */
  SPIx->DR = Data;
}

static inline void spi_i2s_send_data16(SPI_TypeDef* SPIx, uint16_t Data) 
{
  /* Write in the DR register the data to be sent */
  SPIx->DR = Data;
}

/**
  * @brief  Transmits a Data through the SPIx/I2Sx peripheral.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  Data: Data to be transmitted.
  * @retval None
  */
static inline void spi_send_data8(SPI_TypeDef* SPIx, uint8_t Data)
{
  uint32_t spixbase = 0x00;
  spixbase = (uint32_t)SPIx; 
  spixbase += 0x0C;
  *(__IO uint8_t *) spixbase = Data;
}


/**
  * @brief  Enables or disables the CRC value calculation of the transferred bytes.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  NewState: new state of the SPIx CRC value calculation.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void spi_calculate_crc(SPI_TypeDef* SPIx, bool enable)
{
  if (enable)
  {
    /* Enable the selected SPI CRC calculation */
    SPIx->CR1 |= SPI_CR1_CRCEN;
  }
  else
  {
    /* Disable the selected SPI CRC calculation */
    SPIx->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_CRCEN);
  }
}

/**
  * @brief  Transmit the SPIx CRC value.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @retval None
  */
static inline void spi_transmit_crc(SPI_TypeDef* SPIx)
{

  /* Enable the selected SPI CRC transmission */
  SPIx->CR1 |= SPI_CR1_CRCNEXT;
}

/**
  * @brief  Returns the transmit or the receive CRC register value for the specified SPI.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_CRC: specifies the CRC register to be read.
  *          This parameter can be one of the following values:
  *            @arg SPI_CRC_Tx: Selects Tx CRC register
  *            @arg SPI_CRC_Rx: Selects Rx CRC register
  * @retval The selected CRC register value..
  */
static inline uint16_t spi_get_crc(SPI_TypeDef* SPIx, uint8_t SPI_CRC)
{
  uint16_t crcreg;
  if (SPI_CRC != SPI_CRC_Rx)
  {
    /* Get the Tx CRC register */
    crcreg = SPIx->TXCRCR;
  }
  else
  {
    /* Get the Rx CRC register */
    crcreg = SPIx->RXCRCR;
  }
  /* Return the selected CRC register */
  return crcreg;
}

/**
  * @brief  Returns the CRC Polynomial register value for the specified SPI.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @retval The CRC Polynomial register value.
  */
static inline uint16_t spi_get_crc_polynomial(SPI_TypeDef* SPIx)
{
  /* Return the CRC polynomial register */
  return SPIx->CRCPR;
}

/**
  * @brief  Returns the CRC Polynomial register value for the specified SPI.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @retval The CRC Polynomial register value.
  */
static inline void spi_set_crc_polynomial(SPI_TypeDef* SPIx, uint16_t polynomial )
{
  /* Return the CRC polynomial register */
  SPIx->CRCPR = polynomial;
}


/**
  * @brief  Enables or disables the SPIx/I2Sx DMA interface.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_DMAReq: specifies the SPI DMA transfer request to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *            @arg SPI_I2S_DMAReq_Tx: Tx buffer DMA transfer request
  *            @arg SPI_I2S_DMAReq_Rx: Rx buffer DMA transfer request
  * @param  NewState: new state of the selected SPI DMA transfer request.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void spi_i2s_dma_cmd(SPI_TypeDef* SPIx, uint16_t SPI_I2S_DMAReq, bool enable)
{

  if ( enable )
  {
    /* Enable the selected SPI DMA requests */
    SPIx->CR2 |= SPI_I2S_DMAReq;
  }
  else
  {
    /* Disable the selected SPI DMA requests */
    SPIx->CR2 &= (uint16_t)~SPI_I2S_DMAReq;
  }
}

/**
  * @brief  Enables or disables the specified SPI/I2S interrupts.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_IT: specifies the SPI interrupt source to be enabled or disabled.
  *          This parameter can be one of the following values:
  *            @arg SPI_I2S_IT_TXE: Tx buffer empty interrupt mask
  *            @arg SPI_I2S_IT_RXNE: Rx buffer not empty interrupt mask
  *            @arg SPI_I2S_IT_ERR: Error interrupt mask
  * @param  NewState: new state of the specified SPI interrupt.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void spi_i2s_it_config(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT, bool enable )
{
  uint16_t itpos = 0, itmask = 0 ;

  /* Get the SPI IT index */
  itpos = SPI_I2S_IT >> 4;

  /* Set the IT mask */
  itmask = (uint16_t)1 << (uint16_t)itpos;

  if ( enable )
  {
    /* Enable the selected SPI interrupt */
    SPIx->CR2 |= itmask;
  }
  else
  {
    /* Disable the selected SPI interrupt */
    SPIx->CR2 &= (uint16_t)~itmask;
  }
}

/**
  * @brief  Checks whether the specified SPIx/I2Sx flag is set or not.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_FLAG: specifies the SPI flag to check.
  *          This parameter can be one of the following values:
  *            @arg SPI_I2S_FLAG_TXE: Transmit buffer empty flag.
  *            @arg SPI_I2S_FLAG_RXNE: Receive buffer not empty flag.
  *            @arg SPI_I2S_FLAG_BSY: Busy flag.
  *            @arg SPI_I2S_FLAG_OVR: Overrun flag.
  *            @arg SPI_FLAG_MODF: Mode Fault flag.
  *            @arg SPI_FLAG_CRCERR: CRC Error flag.
  *            @arg SPI_I2S_FLAG_TIFRFE: Format Error.
  *            @arg I2S_FLAG_UDR: Underrun Error flag.
  *            @arg I2S_FLAG_CHSIDE: Channel Side flag.
  * @retval The new state of SPI_I2S_FLAG (SET or RESET).
  */
static inline bool spi_i2s_get_flag_status(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG)
{
  /* Return the SPI_I2S_FLAG status */
  return  (SPIx->SR & SPI_I2S_FLAG)?(true):(false);
}

/**
  * @brief  Clears the SPIx CRC Error (CRCERR) flag.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_FLAG: specifies the SPI flag to clear.
  *          This function clears only CRCERR flag.
  *            @arg SPI_FLAG_CRCERR: CRC Error flag.
  *
  * @note   OVR (OverRun error) flag is cleared by software sequence: a read
  *          operation to SPI_DR register (SPI_I2S_ReceiveData()) followed by a read
  *          operation to SPI_SR register (SPI_I2S_GetFlagStatus()).
  * @note   UDR (UnderRun error) flag is cleared by a read operation to
  *          SPI_SR register (SPI_I2S_GetFlagStatus()).
  * @note   MODF (Mode Fault) flag is cleared by software sequence: a read/write
  *          operation to SPI_SR register (SPI_I2S_GetFlagStatus()) followed by a
  *          write operation to SPI_CR1 register (SPI_Cmd() to enable the SPI).
  *
  * @retval None
  */
static inline void spi_i2s_clear_flag(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG)
{

  /* Clear the selected SPI CRC Error (CRCERR) flag */
  SPIx->SR = (uint16_t)~SPI_I2S_FLAG;
}

/**
  * @brief  Checks whether the specified SPIx/I2Sx interrupt has occurred or not.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_IT: specifies the SPI interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg SPI_I2S_IT_TXE: Transmit buffer empty interrupt.
  *            @arg SPI_I2S_IT_RXNE: Receive buffer not empty interrupt.
  *            @arg SPI_I2S_IT_OVR: Overrun interrupt.
  *            @arg SPI_IT_MODF: Mode Fault interrupt.
  *            @arg SPI_IT_CRCERR: CRC Error interrupt.
  *            @arg I2S_IT_UDR: Underrun interrupt.
  *            @arg SPI_I2S_IT_TIFRFE: Format Error interrupt.
  * @retval The new state of SPI_I2S_IT (SET or RESET).
  */
static inline bool spi_i2s_get_it_status(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT)
{
  uint16_t itpos, itmask, enablestatus;

  /* Get the SPI_I2S_IT index */
  itpos = 0x01 << (SPI_I2S_IT & 0x0F);

  /* Get the SPI_I2S_IT IT mask */
  itmask = SPI_I2S_IT >> 4;

  /* Set the IT mask */
  itmask = 0x01 << itmask;

  /* Get the SPI_I2S_IT enable bit status */
  enablestatus = (SPIx->CR2 & itmask) ;

  return ((SPIx->SR & itpos) && enablestatus)?(true):(false);
  /* Check the status of the specified SPI interrupt */
}

/**
  * @brief  Clears the SPIx CRC Error (CRCERR) interrupt pending bit.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2 or 3
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_IT: specifies the SPI interrupt pending bit to clear.
  *         This function clears only CRCERR interrupt pending bit.
  *            @arg SPI_IT_CRCERR: CRC Error interrupt.
  *
  * @note   OVR (OverRun Error) interrupt pending bit is cleared by software
  *          sequence: a read operation to SPI_DR register (SPI_I2S_ReceiveData())
  *          followed by a read operation to SPI_SR register (SPI_I2S_GetITStatus()).
  * @note   UDR (UnderRun Error) interrupt pending bit is cleared by a read
  *          operation to SPI_SR register (SPI_I2S_GetITStatus()).
  * @note   MODF (Mode Fault) interrupt pending bit is cleared by software sequence:
  *          a read/write operation to SPI_SR register (SPI_I2S_GetITStatus())
  *          followed by a write operation to SPI_CR1 register (SPI_Cmd() to enable
  *          the SPI).
  * @retval None
  */
static inline void spi_i2s_clear_it_pending_bit(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT)
{
  uint16_t itpos = 0;

  /* Get the SPI_I2S IT index */
  itpos = 0x01 << (SPI_I2S_IT & 0x0F);

  /* Clear the selected SPI CRC Error (CRCERR) interrupt pending bit */
  SPIx->SR = (uint16_t)~itpos;
}


#ifdef STM32_SPI_V2
/**
  * @brief  Configures the FIFO reception threshold for the selected SPI.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_RxFIFOThreshold: specifies the FIFO reception threshold.
  *          This parameter can be one of the following values:
  *            @arg SPI_RxFIFOThreshold_HF: RXNE event is generated if the FIFO 
  *                                         level is greater or equal to 1/2. 
  *            @arg SPI_RxFIFOThreshold_QF: RXNE event is generated if the FIFO 
  *                                         level is greater or equal to 1/4. 
  * @retval None
  */
static inline void spi_rx_fifo_threshold_config(SPI_TypeDef* SPIx, uint16_t SPI_RxFIFOThreshold)
{
  /* Clear FRXTH bit */
  SPIx->CR2 &= (uint16_t)~((uint16_t)SPI_CR2_FRXTH);
  /* Set new FRXTH bit value */
  SPIx->CR2 |= SPI_RxFIFOThreshold;
}

/**
  * @brief  Configures the CRC calculation length for the selected SPI.
  * @note   This function can be called only after the SPI_Init() function has 
  *         been called.  
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_CRCLength: specifies the SPI CRC calculation length.
  *          This parameter can be one of the following values:
  *            @arg SPI_CRCLength_8b: Set CRC Calculation to 8 bits
  *            @arg SPI_CRCLength_16b: Set CRC Calculation to 16 bits
  * @retval None
  */
static inline void spi_crc_length_config(SPI_TypeDef* SPIx, uint16_t SPI_CRCLength)
{
  /* Clear CRCL bit */
  SPIx->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_CRCL);
  /* Set new CRCL bit value */
  SPIx->CR1 |= SPI_CRCLength;
}


/**
  * @brief  Configures the number of data to transfer type(Even/Odd) for the DMA
  *         last transfers and for the selected SPI.
  * @note   This function have a meaning only if DMA mode is selected and if 
  *         the packing mode is used (data length <= 8 and DMA transfer size halfword)  
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_LastDMATransfer: specifies the SPI last DMA transfers state.
  *          This parameter can be one of the following values:
  *            @arg SPI_LastDMATransfer_TxEvenRxEven: Number of data for transmission Even
  *                                                   and number of data for reception Even.
  *            @arg SPI_LastDMATransfer_TxOddRxEven: Number of data for transmission Odd
  *                                                  and number of data for reception Even.
  *            @arg SPI_LastDMATransfer_TxEvenRxOdd: Number of data for transmission Even
  *                                                  and number of data for reception Odd.
  *            @arg SPI_LastDMATransfer_TxOddRxOdd: Number of data for transmission Odd
  *                                                 and number of data for reception Odd.
  * @retval None
  */
static inline void spi_last_dma_transfer_cmd(SPI_TypeDef* SPIx, uint16_t SPI_LastDMATransfer)
{
  /* Clear LDMA_TX and LDMA_RX bits */
  SPIx->CR2 &= CR2_LDMA_MASK;
  /* Set new LDMA_TX and LDMA_RX bits value */
  SPIx->CR2 |= SPI_LastDMATransfer; 
}



/**
  * @brief  Returns the current SPIx Transmission FIFO filled level.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @retval The Transmission FIFO filling state.
  *          - SPI_TransmissionFIFOStatus_Empty: when FIFO is empty
  *          - SPI_TransmissionFIFOStatus_1QuarterFull: if more than 1 quarter-full.
  *          - SPI_TransmissionFIFOStatus_HalfFull: if more than 1 half-full.
  *          - SPI_TransmissionFIFOStatus_Full: when FIFO is full.
  */
static inline uint16_t spi_get_transmission_fifo_status(SPI_TypeDef* SPIx)
{
  /* Get the SPIx Transmission FIFO level bits */
  return (uint16_t)((SPIx->SR & SPI_SR_FTLVL));
}

/**
  * @brief  Returns the current SPIx Reception FIFO filled level.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @retval The Reception FIFO filling state.
  *          - SPI_ReceptionFIFOStatus_Empty: when FIFO is empty
  *          - SPI_ReceptionFIFOStatus_1QuarterFull: if more than 1 quarter-full.
  *          - SPI_ReceptionFIFOStatus_HalfFull: if more than 1 half-full.
  *          - SPI_ReceptionFIFOStatus_Full: when FIFO is full.
  */
static inline uint16_t spi_get_reception_fifo_status(SPI_TypeDef* SPIx)
{
  /* Get the SPIx Reception FIFO level bits */
  return (uint16_t)((SPIx->SR & SPI_SR_FRLVL));
}


#endif


#ifdef __cplusplus
}
#endif

#undef CR1_CLEAR_MASK
#undef I2SCFGR_CLEAR_MASK
#undef I2S2_CLOCK_SRC
#undef I2S3_CLOCK_SRC
#undef I2S_MUL_MASK
#undef I2S_DIV_MASK
#undef SPI_CR2_FRF
#undef CR2_LDMA_MASK


