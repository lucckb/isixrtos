/*
 * =====================================================================================
 *
 *       Filename:  stm32sdadc.h
 *
 *    Description:  SDADC driver
 *
 *        Version:  1.0
 *        Created:  23.10.2016 18:20:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <stm32lib.h>

#ifndef STM32MCU_MAJOR_TYPE_F37
#error SDADC perhiperal does not exist on selected platform
#endif

#define CR2_CLEAR_MASK            ((uint32_t)0xFE30FFFF)

#ifdef __cplusplus
namespace stm32 {
#endif


/**
  * @brief  Initializes the SDADCx peripheral according to the specified parameters
  *         in the SDADC_InitStruct.
  * @note   SDADC_FastConversionMode can be modified only if the SDADC is disabled
  *         or the INITRDY flag is set. Otherwise the configuration can't be modified.
  * @note   Channel selection and continuous mode configuration affect only the 
  *         regular channel.
  * @note   Fast conversion mode is regardless of regular/injected conversion mode.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_InitStruct: pointer to an SDADC_InitTypeDef structure that contains
  *         the configuration information for the specified SDADC peripheral.
  * @retval None
  */
static inline void sdadc_init(SDADC_TypeDef* SDADCx, 
		uint32_t channel, bool continous_conv_mode, bool fast_conversion_mode )
{
  uint32_t tmpcr2 = 0;


  /*---------------------------- SDADCx CR2 Configuration --------------------*/
  /* Get the SDADCx_CR2 value */
  tmpcr2 = SDADCx->CR2;

  /* Clear FAST, RCONT and RCH bits */
  tmpcr2 &= CR2_CLEAR_MASK;
  /* Configure SDADCx: continuous mode for regular conversion, 
     regular channel and fast conversion mode */
  /* Set RCONT bit according to SDADC_ContinuousConvMode value */
  /* Set FAST bit according to SDADC_FastConversionMode value */
  /* Select the regular channel according to SDADC_Channel value */
  tmpcr2 |= (uint32_t)(((uint32_t)continous_conv_mode<<22) |
                       (fast_conversion_mode<<(uint32_t)24) |
                       (channel & SDADC_CR2_RCH));

  /* Write to SDADCx_CR2 */
  SDADCx->CR2 = tmpcr2;
}



/**
  * @brief  Configures the analog input mode.
  * @note   This function can be used only if the SDADC is disabled 
  *         or the INITRDY flag is set. Otherwise the configuration can't be modified.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_AINStruct: pointer to an SDADC_AINStructTypeDef structure that contains 
  *         the analog inputs configuration information for the specified SDADC peripheral.
  * @retval None
  */
static inline void sdadc_ain_init(SDADC_TypeDef* SDADCx, uint32_t SDADC_Conf,
		uint32_t input_mode, uint32_t gain, uint32_t common_mode, uint32_t offset )
{

  /* Get the ASDACx address */
  uint32_t tmp = (uint32_t)((uint32_t)SDADCx + 0x00000020);
  /* Get the ASDACx CONFxR value: depending SDADC_Conf, analog input configuration
     is set to CONF0R, CONF1R or CONF2R register */
  tmp = (uint32_t)(SDADC_Conf << 2) + tmp;

  /* Set the analog input configuration to the selected CONFxR register */
  *(__IO uint32_t *) (tmp) = (uint32_t) (input_mode | gain |
                                         common_mode | offset);
}

/** Get the AIN offset for selected channel */
static inline int16_t sdadc_ain_offset( SDADC_TypeDef* SDADCx, uint32_t SDADC_Conf ) 
{
  /* Get the ASDACx address */
  uint32_t tmp = (uint32_t)((uint32_t)SDADCx + 0x00000020);
  /* Get the ASDACx CONFxR value: depending SDADC_Conf, analog input configuration
     is set to CONF0R, CONF1R or CONF2R register */
  tmp = (uint32_t)(SDADC_Conf << 2) + tmp;

  /* Set the analog input configuration to the selected CONFxR register 
   * the offset is 12 bit signed integer so the sign is extended*/
  int16_t offs =  *(__IO uint32_t *)(tmp) & 0xfff;
  return ((offs) | (((offs) & (1 << (11))) ? ~((1 << (11))-1) : 0));
}

/**
  * @brief  Configures the SDADCx channel.
  * @note   SDADC channel configuration can be modified only if the SDADC is disabled
  *         or the INITRDY flag is set. Otherwise the configuration can't be modified.  
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_Channel: The SDADC injected channel.
  *          This parameter can be one of the following values:
  *            @arg SDADC_Channel_0: SDADC Channel 0 selected
  *            @arg SDADC_Channel_1: SDADC Channel 1 selected
  *            @arg SDADC_Channel_2: SDADC Channel 2 selected
  *            @arg SDADC_Channel_3: SDADC Channel 3 selected
  *            @arg SDADC_Channel_4: SDADC Channel 4 selected
  *            @arg SDADC_Channel_5: SDADC Channel 5 selected
  *            @arg SDADC_Channel_6: SDADC Channel 6 selected
  *            @arg SDADC_Channel_7: SDADC Channel 7 selected
  *            @arg SDADC_Channel_8: SDADC Channel 8 selected
  * @param  SDADC_Conf: The SDADC input configuration.
  *          This parameter can be one of the following values:
  *            @arg SDADC_Conf_0: SDADC Conf 0 selected
  *            @arg SDADC_Conf_1: SDADC Conf 1 selected
  *            @arg SDADC_Conf_2: SDADC Conf 2 selected
  * @note   The SDADC configuration (Conf 0, Conf 1, Conf 2) should be performed
  *         using SDADC_AINInit()
  * @retval None
  */
static inline void sdadc_channel_config(SDADC_TypeDef* SDADCx, 
		uint32_t SDADC_Channel, uint32_t SDADC_Conf)
{
  uint32_t channelnum = 0;


  /*----------------------- SDADCx CONFCHRx Configuration --------------------*/
  if(SDADC_Channel != SDADC_Channel_8)
  {
    /* Get channel number */
    channelnum = (uint32_t)(SDADC_Channel>>16);
    /* Set the channel configuration */
    SDADCx->CONFCHR1 |= (uint32_t) (SDADC_Conf << (channelnum << 2));
  }
  else
  {
    SDADCx->CONFCHR2 = (uint32_t) (SDADC_Conf);
  }
}



/**
  * @brief  Enables or disables the specified SDADC peripheral.
  * @note   When disabled, power down mode is entered, the flags and the data
  *         are cleared.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the SDADCx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdadc_cmd(SDADC_TypeDef* SDADCx, bool enabled )
{

  if ( enabled )
  {
    /* Set the ADON bit to enable the SDADC */
    SDADCx->CR2 |= (uint32_t)SDADC_CR2_ADON;
  }
  else
  {
    /* Reset the ADON bit to disable the SDADC */
    SDADCx->CR2 &= (uint32_t)(~SDADC_CR2_ADON);
  }
}


/**
  * @brief  Enables or disables the initialization mode for specified SDADC peripheral.
  * @note   Initialization mode should be enabled before setting the analog input
  *         configuration, the fast conversion mode, the external trigger...
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the SDADCx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  *         When enabled, the SDADCx is in initialization mode and the SDADCx can
  *         be configured (except: power down mode, standby mode, slow clock and VREF selection).
  *         When disabled, the SDADCx isn't in initialization mode and limited
  *         configurations are allowed (regular channel selection, software trigger)
  * @retval None
  */
static inline void sdadc_init_mode_cmd(SDADC_TypeDef* SDADCx, bool enabled )
{
  if ( enabled )
  {
    /* Set the INIT bit to enter initialization mode */
    SDADCx->CR1 |= (uint32_t)SDADC_CR1_INIT;
  }
  else
  {
    /* Reset the INIT bit to exit initialization mode */
    SDADCx->CR1 &= (uint32_t)(~SDADC_CR1_INIT);
  }
}


/**
  * @brief  Enables or disables the fast conversion mode for the SDADC.
  * @note   When converting a single channel in continuous mode, having enabled
  *         fast mode causes each conversion (except for the first) to execute
  *         3 times faster (taking 120 SDADC cycles rather than 360).
  *         Fast conversion mode has no meaning for conversions which are not continuous.
  * @note   fast conversion mode can be modified only if the SDADC is disabled
  *         or the INITRDY flag is set. Otherwise the configuration can't be modified.    
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC fast conversion mode
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdadc_fast_conversion_cmd(SDADC_TypeDef* SDADCx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the fast conversion mode */
    SDADCx->CR2 |= SDADC_CR2_FAST;
  }
  else
  {
    /* Disable the fast conversion mode */
    SDADCx->CR2 &= (uint32_t)(~SDADC_CR2_FAST);
  }
}


/**
  * @brief  Selects the reference voltage.
  * @note   The reference voltage is common to the all SDADCs (SDADC1, SDADC2 and SDADC3).
  *         The reference voltage selection is available only in SDADC1 and therefore
  *         to select the VREF for SDADC2/SDADC3, SDADC1 clock must be already enabled.
  * @note   The reference voltage selection can be performed only when the SDADC
  *         is disabled.
  * @param  SDADC_VREF: Reference voltage selection.
  *          This parameter can be one of the following values:
  *            @arg SDADC_VREF_Ext: The reference voltage is forced externally using VREF pin
  *            @arg SDADC_VREF_VREFINT1: The reference voltage is forced internally to 1.22V VREFINT
  *            @arg SDADC_VREF_VREFINT2: The reference voltage is forced internally to 1.8V VREFINT
  *            @arg SDADC_VREF_VDDA: The reference voltage is forced internally to VDDA
  * @retval None
  */
static inline void sdadc_vref_select(uint32_t SDADC_VREF)
{
  uint32_t tmpcr1;
  /* Get SDADC1_CR1 register value */
  tmpcr1 = SDADC1->CR1;

  /* Clear the SDADC1_CR1_REFV bits */
  tmpcr1 &= (uint32_t) (~SDADC_CR1_REFV);
  /* Select the reference voltage */
  tmpcr1 |= SDADC_VREF;

  /* Write in SDADC_CR1 */
  SDADC1->CR1 = tmpcr1;
}


/**
  * @brief  Configures the calibration sequence.
  * @note   After calling SDADC_CalibrationSequenceConfig(), use SDADC_StartCalibration()
  *         to start the calibration process.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_CalibrationSequence: Number of calibration sequence to be performed.
  *          This parameter can be one of the following values:
  *            @arg SDADC_CalibrationSequence_1: One calibration sequence will be performed
  *                                      to calculate OFFSET0[11:0] (offset that corresponds to conf0)
  *            @arg SDADC_CalibrationSequence_2: Two calibration sequences will be performed
  *                                      to calculate OFFSET0[11:0] and OFFSET1[11:0]
  *                                      (offsets that correspond to conf0 and conf1)
  *            @arg SDADC_CalibrationSequence_3: Three calibration sequences will be performed
  *                                      to calculate OFFSET0[11:0], OFFSET1[11:0], 
  *                                      and OFFSET2[11:0] (offsets that correspond to conf0, conf1 and conf2)
  * @retval None
  */
static inline void sdadc_calibration_sequence_config(SDADC_TypeDef* SDADCx, 
		uint32_t SDADC_CalibrationSequence)
{
  uint32_t tmpcr2;

  /* Get SDADC_CR2 register value */
  tmpcr2 = SDADCx->CR2;

  /* Clear the SDADC_CR2_CALIBCNT bits */
  tmpcr2 &= (uint32_t) (~SDADC_CR2_CALIBCNT);
  /* Set the calibration sequence */
  tmpcr2 |= SDADC_CalibrationSequence;

  /* Write in SDADC_CR2 */
  SDADCx->CR2 = tmpcr2;
}


/**
  * @brief  Launches a request to start the calibration sequence.
  * @note   use SDADC_CalibrationSequenceConfig() function to configure the 
  *         calibration sequence then call SDADC_StartCalibration() to start the 
  *         calibration process.  
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval None
  */
static inline void sdadc_start_calibration(SDADC_TypeDef* SDADCx)
{
  /* Request a start of the calibration sequence */
  SDADCx->CR2 |= (uint32_t)SDADC_CR2_STARTCALIB;
}


/**
  * @brief  Selects the SDADC channel to be used for regular conversion.
  * @note   Just one channel of the 9 available channels can be selected.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_Channel: The SDADC regular channel.
  *          This parameter can be one of the following values:
  *            @arg SDADC_Channel_0: SDADC Channel 0 selected
  *            @arg SDADC_Channel_1: SDADC Channel 1 selected
  *            @arg SDADC_Channel_2: SDADC Channel 2 selected
  *            @arg SDADC_Channel_3: SDADC Channel 3 selected
  *            @arg SDADC_Channel_4: SDADC Channel 4 selected
  *            @arg SDADC_Channel_5: SDADC Channel 5 selected
  *            @arg SDADC_Channel_6: SDADC Channel 6 selected
  *            @arg SDADC_Channel_7: SDADC Channel 7 selected
  *            @arg SDADC_Channel_8: SDADC Channel 8 selected
  * @retval None
  */
static inline void sdadc_channel_select(SDADC_TypeDef* SDADCx, uint32_t SDADC_Channel)
{
	uint32_t tmpcr2;

  /* Get SDADC_CR2 register value */
  tmpcr2 = SDADCx->CR2;

  /* Clear the RCH[3:0] bits */
  tmpcr2 &= (uint32_t) (~SDADC_CR2_RCH);
  /* Select the regular channel */
  tmpcr2 |= (uint32_t) (SDADC_Channel & 0xFFFF0000);

  /* Write in SDADC_CR2 register */
  SDADCx->CR2 = tmpcr2;
}


/**
  * @brief  Enables or disables the SDADC continuous conversion mode.
  *         When enabled, the regular channel is converted repeatedly after each
  *         conversion request.
  *         When disabled, the regular channel is converted once for each
  *         conversion request.     
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC continuous conversion mode
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdadc_continuous_mode_cmd(SDADC_TypeDef* SDADCx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the selected SDADC continuous conversion mode */
    SDADCx->CR2 |= (uint32_t)SDADC_CR2_RCONT;
  }
  else
  {
    /* Disable the selected SDADC continuous conversion mode */
    SDADCx->CR2 &= (uint32_t)(~SDADC_CR2_RCONT);
  }
}


/**
  * @brief  Enables the selected SDADC software start conversion of the regular channels.
  * @note   If the flag SDADC_FLAG_RCIP is set or INIT bit is set, calling this
  *         function SDADC_SoftwareStartConv() has no effect.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval None
  */
static inline void sdadc_software_start_conv(SDADC_TypeDef* SDADCx)
{
  /* Enable the selected SDADC conversion for regular group */
  SDADCx->CR2 |= (uint32_t)SDADC_CR2_RSWSTART;
}


/**
  * @brief  Returns the last SDADC conversion result data for regular channel.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval The Data conversion value.
  */
static inline int16_t sdadc_get_conversion_value(SDADC_TypeDef* SDADCx)
{
  /* Return the selected SDADC conversion value for regular channel */
  return (int16_t) SDADCx->RDATAR;
}


/**
  * @brief  Launches SDADC2/SDADC3 regular conversion synchronously with SDADC1.
  * @note   This feature is available only on SDADC2 and SDADC3.
  * @param  SDADCx: where x can be 2 or 3 to select the SDADC peripheral.
  *         When enabled, a regular conversion is launched at the same moment
  *         that a regular conversion is launched in SDADC1.
  *         When disabled, do not launch a regular conversion synchronously with SDADC1.
  * @retval None
  */
static inline void sdadc_regular_synchro_sdadc1(SDADC_TypeDef* SDADCx, bool en )
{
  if ( en )
  {
    /* Enable synchronization with SDADC1 on regular conversions */
    SDADCx->CR1 |= SDADC_CR1_RSYNC;
  }
  else
  {
    /* Disable synchronization with SDADC1 on regular conversions */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_RSYNC;
  }
}


/**
  * @brief  Returns the last conversion result data for regular channel of SDADC1 and SDADC2.
  *         RSYNC bit of the SDADC2 should be already set.
  * @param  None
  * @retval The Data conversion value for SDADC1 and SDADC2.
  *         In 16-bit MSB: the regular conversion data for SDADC2.
  *          This data is valid only when the flag SDADC_FLAG_REOC of SDADC2 is set.  
  *         In 16-bit LSB: the regular conversion data for SDADC1.
  *          This data is valid only when the flag SDADC_FLAG_REOC of SDADC1 is set.    
  */
static inline uint32_t sdadc_get_conversion_sdadc12_value(void)
{
  /* Return the selected conversion value for regular channel of SDADC1 and SDADC2*/
  return (uint32_t) SDADC1->RDATA12R;
}


/**
  * @brief  Returns the last conversion result data for regular channel of SDADC1 and SDADC3.
  *         RSYNC bit of the SDADC3 should be already set.
  * @param  None
  * @retval The Data conversion value for SDADC1 and SDADC3.
  *         In 16-bit MSB: the regular conversion data for SDADC3.
  *          This data is valid only when the flag SDADC_FLAG_REOC of SDADC3 is set.  
  *         In 16-bit LSB: the regular conversion data for SDADC1.
  *          This data is valid only when the flag SDADC_FLAG_REOC of SDADC1 is set.
  */
static inline uint32_t sdadc_get_conversion_sdadc13_value(void)
{
  /* Return the selected conversion value for regular channel of SDADC1 and SDADC3*/
  return (uint32_t) SDADC1->RDATA13R;
}


/**
  * @brief  Enables the selected SDADC software start conversion of the injected 
  *         channels.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval None
  */
static inline void sdadc_software_startinjected_conv(SDADC_TypeDef* SDADCx)
{
  /* Start a conversion of the injected group of channels */
  SDADCx->CR2 |= (uint32_t)SDADC_CR2_JSWSTART;
}


/**
  * @brief  Selects the SDADC injected channel(s).
  * @note   When selected, the SDADC channel is part of the injected group
  *         By default, channel 0 is selected
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_Channel: The SDADC injected channel.
  *          This parameter can be one or any combination of the following values:
  *            @arg SDADC_Channel_0: SDADC Channel 0 selected
  *            @arg SDADC_Channel_1: SDADC Channel 1 selected
  *            @arg SDADC_Channel_2: SDADC Channel 2 selected
  *            @arg SDADC_Channel_3: SDADC Channel 3 selected
  *            @arg SDADC_Channel_4: SDADC Channel 4 selected
  *            @arg SDADC_Channel_5: SDADC Channel 5 selected
  *            @arg SDADC_Channel_6: SDADC Channel 6 selected
  *            @arg SDADC_Channel_7: SDADC Channel 7 selected
  *            @arg SDADC_Channel_8: SDADC Channel 8 selected
  * @retval None
  */
static inline void sdadc_injected_channel_select(SDADC_TypeDef* SDADCx, uint32_t SDADC_Channel)
{
  /* Select the SDADC injected channel */
  SDADCx->JCHGR = (uint32_t) (SDADC_Channel & 0x0000FFFF);
}


/**
  * @brief  Enables or disables delayed start of injected conversions
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC delay start of injected
  *         conversions. This parameter can be: ENABLE or DISABLE.
  *         When disabled, injected conversions begin as soon as possible after
  *         the request.
  *         When enabled, after a request for injected conversion the SDADC waits
  *         a fixed interval before launching the conversion.
  * @retval None
  */
static inline void sdadc_delay_start_injected_conv_cmd(SDADC_TypeDef* SDADCx, bool en )
{

  if ( en )
  {
    /* Enable delay start of injected conversions */
    SDADCx->CR2 |= (uint32_t) (SDADC_CR2_JDS);
  }
  else
  {
    /* Disable delay start of injected conversions */
    SDADCx->CR2 &= (uint32_t) ~(SDADC_CR2_JDS);
  }
}


/**
  * @brief  Enables or disables the continuous mode for injected channels for
  *         the specified SDADC
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC continuous mode
  *         on injected channels. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdadc_injected_continuous_mode_cmd(SDADC_TypeDef* SDADCx, bool en)
{
  if (en)
  {
    /* Enable the SDADC continuous mode for injected channels */
    SDADCx->CR2 |= (uint32_t)SDADC_CR2_JCONT;
  }
  else
  {
    /* Disable the SDADC continuous mode for injected channels */
    SDADCx->CR2 &= (uint32_t)(~SDADC_CR2_JCONT);
  }
}


/**
  * @brief  Configures the SDADCx external trigger for injected channels conversion.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_ExternalTrigInjecConv: specifies the SDADC trigger to start injected 
  *    conversion. This parameter can be one of the following values:
  *            @arg SDADC_ExternalTrigInjecConv_T13_CC1: Timer13 capture compare1 selected 
  *            @arg SDADC_ExternalTrigInjecConv_T14_CC1: Timer14 TRGO event selected 
  *            @arg SDADC_ExternalTrigInjecConv_T16_CC1: Timer16 TRGO event selected
  *            @arg SDADC_ExternalTrigInjecConv_T17_CC1: Timer17 capture compare1 selected
  *            @arg SDADC_ExternalTrigInjecConv_T12_CC1: Timer12 capture compare1 selected
  *            @arg SDADC_ExternalTrigInjecConv_T12_CC2: Timer12 capture compare2 selected
  *            @arg SDADC_ExternalTrigInjecConv_T15_CC2: Timer15 capture compare2 selected
  *            @arg SDADC_ExternalTrigInjecConv_T2_CC3: Timer2 capture compare3 selected
  *            @arg SDADC_ExternalTrigInjecConv_T2_CC4: Timer2 capture compare4 selected
  *            @arg SDADC_ExternalTrigInjecConv_T3_CC1: Timer3 capture compare1 selected
  *            @arg SDADC_ExternalTrigInjecConv_T3_CC2: Timer3 capture compare2 selected
  *            @arg SDADC_ExternalTrigInjecConv_T3_CC3: Timer3 capture compare3 selected
  *            @arg SDADC_ExternalTrigInjecConv_T4_CC1: Timer4 capture compare1 selected
  *            @arg SDADC_ExternalTrigInjecConv_T4_CC2: Timer4 capture compare2 selected
  *            @arg SDADC_ExternalTrigInjecConv_T4_CC3: Timer4 capture compare3 selected
  *            @arg SDADC_ExternalTrigInjecConv_T19_CC2: Timer19 capture compare2 selected 
  *            @arg SDADC_ExternalTrigInjecConv_T19_CC3: Timer19 capture compare3 selected
  *            @arg SDADC_ExternalTrigInjecConv_T19_CC4: Timer19 capture compare4 selected
  *            @arg SDADC_ExternalTrigInjecConv_T4_CC4: Timer4 capture compare4 selected
  *            @arg SDADC_ExternalTrigInjecConv_Ext_IT11: External interrupt line 11 event selected
  *            @arg SDADC_ExternalTrigInjecConv_Ext_IT15: External interrupt line 15 event selected
  * @retval None
  */
static inline void sdadc_external_trig_injected_conv_config(SDADC_TypeDef* SDADCx, 
		uint32_t SDADC_ExternalTrigInjecConv)
{
  uint32_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = SDADCx->CR2;

  /* Clear the old external trigger selection for injected group */
  tmpreg &= (uint32_t) (~SDADC_CR2_JEXTSEL);
  /* Set the external event selection for injected group */
  tmpreg |= SDADC_ExternalTrigInjecConv;

  /* Store the new register value */
  SDADCx->CR2 = tmpreg;
}


/**
  * @brief  Configures the SDADCx external trigger edge for injected channels conversion.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_ExternalTrigInjecConvEdge: specifies the SDADC external trigger
  *         edge to start injected conversion.
  *          This parameter can be one of the following values:
  *            @arg SDADC_ExternalTrigInjecConvEdge_None: external trigger disabled for
  *          injected conversion
  *            @arg SDADC_ExternalTrigInjecConvEdge_Rising: Each rising edge on the selected
  *          trigger makes a request to launch a injected conversion
  *            @arg SDADC_ExternalTrigInjecConvEdge_Falling: Each falling edge on the selected
  *          trigger makes a request to launch a injected conversion
  *            @arg SDADC_ExternalTrigInjecConvEdge_RisingFalling: Both rising edges and 
  *          falling edges on the selected trigger make requests to launch injected conversions.
  * @retval None
  */
static inline void sdadc_external_trig_injected_conv_edge_config(SDADC_TypeDef* SDADCx, 
		uint32_t SDADC_ExternalTrigInjecConvEdge)
{
  uint32_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = SDADCx->CR2;

  /* Clear the old external trigger edge for injected group */
  tmpreg &= (uint32_t) (~SDADC_CR2_JEXTEN);
  /* Set the new external trigger edge for injected group */
  tmpreg |= SDADC_ExternalTrigInjecConvEdge;

  /* Store the new register value */
  SDADCx->CR2 = tmpreg;
}


/**
  * @brief  Returns the injected channel most recently converted for
  *         the specified SDADC
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval The most recently converted SDADC injected channel.
  *          This parameter can be one of the following values:
  *            @arg 0x00000001: SDADC_Channel_0 is recently converted
  *            @arg 0x00010002: SDADC_Channel_1 is recently converted
  *            @arg 0x00020004: SDADC_Channel_2 is recently converted
  *            @arg 0x00030008: SDADC_Channel_3 is recently converted
  *            @arg 0x00040010: SDADC_Channel_4 is recently converted
  *            @arg 0x00050020: SDADC_Channel_5 is recently converted
  *            @arg 0x00060040: SDADC_Channel_6 is recently converted
  *            @arg 0x00070080: SDADC_Channel_7 is recently converted
  *            @arg 0x00080100: SDADC_Channel_8 is recently converted
  */
static inline uint32_t sdadc_get_injected_channel(SDADC_TypeDef* SDADCx)
{
  uint32_t temp = 0;
  
  /* Get the injected channel most recently converted */
  temp = (uint32_t)(SDADCx->JDATAR>>16);
  temp = (uint32_t) (((uint32_t)1<<temp) | (temp<<(uint32_t)16));

  /* Returns the injected channel most recently converted */
  return (uint32_t) (temp);
}


/**
  * @brief  Returns the SDADC injected channel conversion result
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_Channel: the most recently converted SDADC injected channel.
  *          This parameter can be one of the following values:
  *            @arg 0x00000001: SDADC_Channel_0 is recently converted 
  *            @arg 0x00010002: SDADC_Channel_1 is recently converted
  *            @arg 0x00020004: SDADC_Channel_2 is recently converted
  *            @arg 0x00030008: SDADC_Channel_3 is recently converted
  *            @arg 0x00040010: SDADC_Channel_4 is recently converted
  *            @arg 0x00050020: SDADC_Channel_5 is recently converted
  *            @arg 0x00060040: SDADC_Channel_6 is recently converted
  *            @arg 0x00070080: SDADC_Channel_7 is recently converted
  *            @arg 0x00080100: SDADC_Channel_8 is recently converted
  * @retval The injected data conversion value.
  */
static inline int16_t sdadc_get_injected_conversion_value(SDADC_TypeDef* SDADCx, 
		uint32_t* SDADC_Channel)
{
  uint32_t tmp = 0;


  /* Get SDADC_JDATAR register */
  tmp = (uint32_t)SDADCx->JDATAR;

  /* Get the injected channel most recently converted */
   *(uint32_t*)SDADC_Channel = (uint32_t) ((uint32_t)((tmp>>8) &0xffff0000) | (((uint32_t)1<<(tmp>>24))));

  /* Returns the injected channel conversion data */
  return (int16_t) ((uint32_t)(tmp & 0x0000FFFF));
}



/**
  * @brief  Launches injected conversion synchronously with SDADC1.
  * @note   This feature is available only on SDADC2 and SDADC3.
  * @param  SDADCx: where x can be 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC synchronization with SDADC1
  *          This parameter can be: ENABLE or DISABLE.
  *         When enabled, An injected conversion is launched at the same moment
  *         that an injected conversion is launched in SDADC1.
  *         When disabled, do not launch an injected conversion synchronously with SDADC1.
  * @retval None
  */
static inline void sdadc_injected_synchro_sdadc1(SDADC_TypeDef* SDADCx, bool en )
{
  if ( en )
  {
    /* Enable synchronization with SDADC1 on injected conversions */
    SDADCx->CR1 |= SDADC_CR1_JSYNC;
  }
  else
  {
    /* Disable synchronization with SDADC1 on injected conversions */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_JSYNC;
  }
}


/**
  * @brief  Returns the last conversion result data for injected channel of SDADC1 and SDADC2.
  *         JSYNC bit of the SDADC2 should be already set.
  * @param  None
  * @retval The Data conversion value for SDADC1 and SDADC2.
  *         In 16-bit MSB: the regular conversion data for SDADC2.
  *          This data is valid only when the flag SDADC_FLAG_JEOC of SDADC2 is set.  
  *         In 16-bit LSB: the regular conversion data for SDADC1.
  *          This data is valid only when the flag SDADC_FLAG_JEOC of SDADC1 is set.    
  */
static inline uint32_t sdadc_get_injected_conversion_sdadc12_value(void)
{
  /* Return the selected conversion value for injected channel of SDADC1 and SDADC2*/
  return (uint32_t) SDADC1->JDATA12R;
}

/**
  * @brief  Returns the last conversion result data for injected channel of SDADC1 and SDADC3.
  *         JSYNC bit of the SDADC3 should be already set.
  * @param  None
  * @retval The Data conversion value for SDADC1 and SDADC3.
  *         In 16-bit MSB: the injected conversion data for SDADC3.
  *          This data is valid only when the flag SDADC_FLAG_JEOC of SDADC3 is set.  
  *         In 16-bit LSB: the injected conversion data for SDADC1.
  *          This data is valid only when the flag SDADC_FLAG_JEOC of SDADC1 is set.
  */
static inline uint32_t sdadc_get_injected_conversion_sdadc13_value(void)
{
  /* Return the selected conversion value for injected channel of SDADC1 and SDADC3*/
  return (uint32_t) SDADC1->JDATA13R;
}


/**
  * @brief  Enables or disables the SDADC power down mode when idle.
  * @note   SDADC power down mode when idle is used to cut the consumption when
  *         the SDADC is not converting (when idle).
  * @note   When the SDADC is in power down mode and a conversion is requested, 
  *         the SDADC takes 100us to stabilize before launching the conversion.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.  
  * @param  NewState: new state of the selected SDADC power down mode when idle
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdadc_power_down_cmd(SDADC_TypeDef* SDADCx, bool en )
{
  if ( en )
  {
    /* Enable the SDADC power-down when idle */
    SDADCx->CR1 |= SDADC_CR1_PDI;
  }
  else
  {
    /* Disable the SDADCx power-down when idle */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_PDI;
  }
}


/**
  * @brief  Enables or disables the SDADC standby mode when idle.
  * @note   SDADC standby mode when idle is used to cut the consumption when
  *         the SDADC is not converting (when idle).
  * @note   When the SDADC is in standby mode and a conversion is requested, 
  *         the SDADC takes 50us to stabilize before launching the conversion.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.  
  * @param  NewState: new state of the selected SDADC standby mode when idle
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdadc_standby_cmd(SDADC_TypeDef* SDADCx, bool en )
{
  if ( en )
  {
    /* Enable the standby mode when idle */
    SDADCx->CR1 |= SDADC_CR1_SBI;
  }
  else
  {
    /* Disable the standby mode when idle */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_SBI;
  }
}


/**
  * @brief  Enables or disables the SDADC in slow clock mode.
  * @note   Slow clock mode (where the SDADC clock frequency should be 1.5MHz)
  *         allowing a lower level of current consumption as well as operation
  *         at a lower minimum voltage.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.  
  * @param  NewState: new state of the selected SDADC slow clock mode
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdadc_slow_clock_cmd(SDADC_TypeDef* SDADCx, bool en )
{
  if ( en )
  {
    /* Enable the slow clock mode */
    SDADCx->CR1 |= SDADC_CR1_SLOWCK;
  }
  else
  {
    /* Disable the slow clock mode */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_SLOWCK;
  }
}


/**
  * @brief  Configures the DMA transfer for regular/injected conversions.  
  * @note   DMA requests can't be enabled for both regular and injected conversions.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_DMATransfer: Specifies the SDADC DMA transfer.
  *          This parameter can be one of the following values:
  *          @arg SDADC_DMATransfer_Regular: When enabled, DMA manages reading the 
  *          data for the regular channel.
  *          @arg SDADC_DMATransfer_Injected: When enabled, DMA manages reading the
  *          data for the injected channel.
  * @param  NewState Indicates the new state of the SDADC DMA interface.
  *           This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
static inline void sdadc_dma_config(SDADC_TypeDef* SDADCx, uint32_t SDADC_DMATransfer, bool en )
{
  if ( en )
  {
    /* Enable the DMA transfer */
    SDADCx->CR1 |= SDADC_DMATransfer;
  }
  else
  {
    /* Disable the DMA transfer */
    SDADCx->CR1 &= (uint32_t)(~SDADC_DMATransfer);
  }
}


/**
  * @brief  Enables or disables the specified SDADC interrupts.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_IT: specifies the SDADC interrupt sources to be enabled or disabled. 
  *          This parameter can be one of the following values:
  *            @arg SDADC_IT_EOCAL: End of calibration interrupt
  *            @arg SDADC_IT_JEOC: End of injected conversion interrupt
  *            @arg SDADC_IT_JOVR: Injected conversion overrun interrupt
  *            @arg SDADC_IT_REOC: End of regular conversion interrupt
  *            @arg SDADC_IT_ROVR: Regular conversion overrun interrupt
  * @param  NewState: new state of the specified SDADC interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdadc_it_config(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT, bool en )
{
  if ( en )
  {
    /* Enable the selected SDADC interrupts */
    SDADCx->CR1 |= SDADC_IT;
  }
  else
  {
    /* Disable the selected SDADC interrupts */
    SDADCx->CR1 &= ((uint32_t)~SDADC_IT);
  }
}


/**
  * @brief  Checks whether the specified SDADC flag is set or not.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_FLAG: specifies the flag to check. 
  *          This parameter can be one of the following values:
  *            @arg SDADC_FLAG_EOCAL: End of calibration flag
  *            @arg SDADC_FLAG_JEOC: End of injected conversion flag
  *            @arg SDADC_FLAG_JOVR: Injected conversion overrun flag
  *            @arg SDADC_FLAG_REOC: End of regular conversion flag
  *            @arg SDADC_FLAG_ROVR: Regular conversion overrun flag
  *            @arg SDADC_FLAG_CALIBIP:Calibration in progress status flag
  *            @arg SDADC_FLAG_JCIP: Injected conversion in progress status flag
  *            @arg SDADC_FLAG_RCIP: Regular conversion in progress status flag
  *            @arg SDADC_FLAG_STABIP: Stabilization in progress status flag
  *            @arg SDADC_FLAG_INITRDY: Initialization mode is ready
  * @retval The new state of SDADC_FLAG (SET or RESET).
  */
static inline bool sdadc_get_flag_status(SDADC_TypeDef* SDADCx, uint32_t SDADC_FLAG)
{
  /* Check the status of the specified SDADC flag */
  return ((SDADCx->ISR & SDADC_FLAG) != (uint32_t)0);
}


/**
  * @brief  Clears the SDADCx pending flags.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg SDADC_FLAG_EOCAL: End of calibration flag
  *            @arg SDADC_FLAG_JOVR: Injected conversion overrun flag
  *            @arg SDADC_FLAG_ROVR: Regular conversion overrun flag
  * @retval None
  */
static inline void sdadc_clear_flag(SDADC_TypeDef* SDADCx, uint32_t SDADC_FLAG)
{
  /* Clear the selected SDADC flags */
  SDADCx->CLRISR |= (uint32_t)SDADC_FLAG;
}


/**
  * @brief  Checks whether the specified SDADC interrupt has occurred or not.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_IT: specifies the SDADC interrupt source to check. 
  *          This parameter can be one of the following values:
  *            @arg SDADC_IT_EOCAL: End of calibration flag 
  *            @arg SDADC_IT_JEOC: End of injected conversion flag
  *            @arg SDADC_IT_JOVR: Injected conversion overrun flag
  *            @arg SDADC_IT_REOC: End of regular conversion flag
  *            @arg SDADC_IT_ROVR: Regular conversion overrun flag
  * @retval The new state of SDADC_IT (SET or RESET).
  */
static inline bool sdadc_get_it_status(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT)
{
  uint32_t itstatus , enablestatus ;
  /* Get the SDADC interrupt pending status */
  itstatus = (uint32_t) (SDADC_IT & SDADCx->ISR);
  /* Get the SDADC IT enable bit status */
  enablestatus = (SDADCx->CR1 & (uint32_t)SDADC_IT);

  /* Check the status of the specified SDADC interrupt */
  return ((itstatus != (uint32_t)RESET) && (enablestatus != (uint32_t)0));
}


/**
  * @brief  Clears the SDADCx interrupt pending bits.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_IT: specifies the SDADC interrupt pending bit to clear.
  *          This parameter can be any combination of the following values:
  *            @arg SDADC_IT_EOCAL: End of calibration flag 
  *            @arg SDADC_IT_JOVR: Injected conversion overrun flag
  *            @arg SDADC_IT_ROVR: Regular conversion overrun flag
  * @retval None
  */
static inline void sdadc_clear_it_pending_bit(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT)
{
  /* Clear the selected SDADC interrupt pending bits */
  SDADCx->CLRISR |= (uint32_t)SDADC_IT;
}


#ifdef __cplusplus
}
#endif

#undef CR2_CLEAR_MASK
