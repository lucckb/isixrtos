/*
 * 	stm32adc.h
 *	STM32 adc converter for F4 platform
 *  Created on: 23-07-2012
 *      Author: lucck
 */

#ifndef STM32F4ADC_H_
#define STM32F4ADC_H_
/* ---------------------------------------------------------------------------- */
#include <stm32lib.h>
/* ---------------------------------------------------------------------------- */
/* ADC DISCNUM mask */
#define CR1_DISCNUM_RESET         ((uint32_t)0xFFFF1FFF)
/* ADC AWDCH mask */
#define CR1_AWDCH_RESET           ((uint32_t)0xFFFFFFE0)
/* ADC Analog watchdog enable mode mask */
#define CR1_AWDMode_RESET         ((uint32_t)0xFF3FFDFF)
/* CR1 register Mask */
#define CR1_CLEAR_MASK            ((uint32_t)0xFCFFFEFF)
/* ADC EXTEN mask */
#define CR2_EXTEN_RESET           ((uint32_t)0xCFFFFFFF)
/* ADC JEXTEN mask */
#define CR2_JEXTEN_RESET          ((uint32_t)0xFFCFFFFF)
/* ADC JEXTSEL mask */
#define CR2_JEXTSEL_RESET         ((uint32_t)0xFFF0FFFF)
/* CR2 register Mask */
#define CR2_CLEAR_MASK            ((uint32_t)0xC0FFF7FD)
/* ADC SQx mask */
#define SQR3_SQ_SET               ((uint32_t)0x0000001F)
#define SQR2_SQ_SET               ((uint32_t)0x0000001F)
#define SQR1_SQ_SET               ((uint32_t)0x0000001F)
/* ADC L Mask */
#define SQR1_L_RESET              ((uint32_t)0xFF0FFFFF)
/* ADC JSQx mask */
#define JSQR_JSQ_SET              ((uint32_t)0x0000001F)
/* ADC JL mask */
#define JSQR_JL_SET               ((uint32_t)0x00300000)
#define JSQR_JL_RESET             ((uint32_t)0xFFCFFFFF)
/* ADC SMPx mask */
#define SMPR1_SMP_SET             ((uint32_t)0x00000007)
#define SMPR2_SMP_SET             ((uint32_t)0x00000007)
/* ADC JDRx registers offset */
#define JDR_OFFSET                ((uint8_t)0x28)
/* ADC CDR register base address */
#define CDR_ADDRESS               ((uint32_t)0x40012308)
/* ADC CCR register Mask */
#define CR_CLEAR_MASK             ((uint32_t)0xFFFC30E0)

/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
 namespace stm32 {
#endif
/* ---------------------------------------------------------------------------- */
 /**
   * @brief  Initializes the ADCx peripheral according to the specified parameters
   *         in the ADC_InitStruct.
   * @note   This function is used to configure the global features of the ADC (
   *         Resolution and Data Alignment), however, the rest of the configuration
   *         parameters are specific to the regular channels group (scan mode
   *         activation, continuous mode activation, External trigger source and
   *         edge, number of conversion in the regular channels group sequencer).
   * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
   * @param  ADC_InitStruct: pointer to an ADC_InitTypeDef structure that contains
   *         the configuration information for the specified ADC peripheral.
   * @retval None
   */
 static inline void adc_init(ADC_TypeDef* ADCx, uint32_t adc_resolution, bool scan_conv_mode,
		 bool continous_conv_mode, uint32_t external_trig_conv_edge,  uint32_t external_trig_conv,
		 uint32_t data_align, uint8_t nbr_of_conversion)
 {
   uint32_t tmpreg1 = 0;
   uint8_t tmpreg2 = 0;

   /*---------------------------- ADCx CR1 Configuration -----------------*/
   /* Get the ADCx CR1 value */
   tmpreg1 = ADCx->CR1;

   /* Clear RES and SCAN bits */
   tmpreg1 &= CR1_CLEAR_MASK;

   /* Configure ADCx: scan conversion mode and resolution */
   /* Set SCAN bit according to ADC_ScanConvMode value */
   /* Set RES bit according to ADC_Resolution value */
   tmpreg1 |= (uint32_t)(((uint32_t)scan_conv_mode << 8) | \
                                    adc_resolution);
   /* Write to ADCx CR1 */
   ADCx->CR1 = tmpreg1;
   /*---------------------------- ADCx CR2 Configuration -----------------*/
   /* Get the ADCx CR2 value */
   tmpreg1 = ADCx->CR2;

   /* Clear CONT, ALIGN, EXTEN and EXTSEL bits */
   tmpreg1 &= CR2_CLEAR_MASK;

   /* Configure ADCx: external trigger event and edge, data alignment and
      continuous conversion mode */
   /* Set ALIGN bit according to ADC_DataAlign value */
   /* Set EXTEN bits according to ADC_ExternalTrigConvEdge value */
   /* Set EXTSEL bits according to ADC_ExternalTrigConv value */
   /* Set CONT bit according to ADC_ContinuousConvMode value */
   tmpreg1 |= (uint32_t)(data_align | \
                         external_trig_conv |
                         external_trig_conv_edge | \
                         ((uint32_t)continous_conv_mode << 1));

   /* Write to ADCx CR2 */
   ADCx->CR2 = tmpreg1;
   /*---------------------------- ADCx SQR1 Configuration -----------------*/
   /* Get the ADCx SQR1 value */
   tmpreg1 = ADCx->SQR1;

   /* Clear L bits */
   tmpreg1 &= SQR1_L_RESET;

   /* Configure ADCx: regular channel sequence length */
   /* Set L bits according to ADC_NbrOfConversion value */
   tmpreg2 |= (uint8_t)(nbr_of_conversion - (uint8_t)1);
   tmpreg1 |= ((uint32_t)tmpreg2 << 20);

   /* Write to ADCx SQR1 */
   ADCx->SQR1 = tmpreg1;
 }
/* ---------------------------------------------------------------------------- */
 /**
   * @brief  Initializes the ADCs peripherals according to the specified parameters
   *         in the ADC_CommonInitStruct.
   * @param  ADC_CommonInitStruct: pointer to an ADC_CommonInitTypeDef structure
   *         that contains the configuration information for  All ADCs peripherals.
   * @retval None
   */
 static inline void adc_common_init(uint32_t mode, uint32_t prescaler,
		 uint32_t dma_access_mode, uint32_t two_sampling_delay)
 {
   uint32_t tmpreg1 = 0;

   /*---------------------------- ADC CCR Configuration -----------------*/
   /* Get the ADC CCR value */
   tmpreg1 = ADC->CCR;

   /* Clear MULTI, DELAY, DMA and ADCPRE bits */
   tmpreg1 &= CR_CLEAR_MASK;

   /* Configure ADCx: Multi mode, Delay between two sampling time, ADC prescaler,
      and DMA access mode for multimode */
   /* Set MULTI bits according to ADC_Mode value */
   /* Set ADCPRE bits according to ADC_Prescaler value */
   /* Set DMA bits according to ADC_DMAAccessMode value */
   /* Set DELAY bits according to ADC_TwoSamplingDelay value */
   tmpreg1 |= (uint32_t)(mode | prescaler | dma_access_mode | two_sampling_delay);

   /* Write to ADC CCR */
   ADC->CCR = tmpreg1;
 }
/* ---------------------------------------------------------------------------- */
 /**
   * @brief  Enables or disables the specified ADC peripheral.
   * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
   * @param  NewState: new state of the ADCx peripheral.
   *          This parameter can be: ENABLE or DISABLE.
   * @retval None
   */
 static inline void adc_cmd(ADC_TypeDef* ADCx, bool enable)
 {
   if ( enable )
   {
     /* Set the ADON bit to wake up the ADC from power down mode */
     ADCx->CR2 |= (uint32_t)ADC_CR2_ADON;
   }
   else
   {
     /* Disable the selected ADC peripheral */
     ADCx->CR2 &= (uint32_t)(~ADC_CR2_ADON);
   }
 }
/* ---------------------------------------------------------------------------- */
 /**
   * @brief  Enables or disables the analog watchdog on single/all regular or
   *         injected channels
   * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
   * @param  ADC_AnalogWatchdog: the ADC analog watchdog configuration.
   *         This parameter can be one of the following values:
   *            @arg ADC_AnalogWatchdog_SingleRegEnable: Analog watchdog on a single regular channel
   *            @arg ADC_AnalogWatchdog_SingleInjecEnable: Analog watchdog on a single injected channel
   *            @arg ADC_AnalogWatchdog_SingleRegOrInjecEnable: Analog watchdog on a single regular or injected channel
   *            @arg ADC_AnalogWatchdog_AllRegEnable: Analog watchdog on all regular channel
   *            @arg ADC_AnalogWatchdog_AllInjecEnable: Analog watchdog on all injected channel
   *            @arg ADC_AnalogWatchdog_AllRegAllInjecEnable: Analog watchdog on all regular and injected channels
   *            @arg ADC_AnalogWatchdog_None: No channel guarded by the analog watchdog
   * @retval None
   */
 static inline void adc_analog_watchdog_cmd(ADC_TypeDef* ADCx, uint32_t analog_watchdog)
 {
   uint32_t tmpreg = 0;

   /* Get the old register value */
   tmpreg = ADCx->CR1;

   /* Clear AWDEN, JAWDEN and AWDSGL bits */
   tmpreg &= CR1_AWDMode_RESET;

   /* Set the analog watchdog enable mode */
   tmpreg |= analog_watchdog;

   /* Store the new register value */
   ADCx->CR1 = tmpreg;
 }
/* ---------------------------------------------------------------------------- */
 /**
   * @brief  Configures the high and low thresholds of the analog watchdog.
   * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
   * @param  HighThreshold: the ADC analog watchdog High threshold value.
   *          This parameter must be a 12-bit value.
   * @param  LowThreshold:  the ADC analog watchdog Low threshold value.
   *          This parameter must be a 12-bit value.
   * @retval None
   */
static inline void adc_analog_watchdog_thresholds_config(ADC_TypeDef* ADCx,
		 uint16_t HighThreshold, uint16_t LowThreshold)
{
   /* Set the ADCx high threshold */
   ADCx->HTR = HighThreshold;

   /* Set the ADCx low threshold */
   ADCx->LTR = LowThreshold;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures the analog watchdog guarded single channel
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure for the analog watchdog.
  *          This parameter can be one of the following values:
  *            @arg ADC_Channel_0: ADC Channel0 selected
  *            @arg ADC_Channel_1: ADC Channel1 selected
  *            @arg ADC_Channel_2: ADC Channel2 selected
  *            @arg ADC_Channel_3: ADC Channel3 selected
  *            @arg ADC_Channel_4: ADC Channel4 selected
  *            @arg ADC_Channel_5: ADC Channel5 selected
  *            @arg ADC_Channel_6: ADC Channel6 selected
  *            @arg ADC_Channel_7: ADC Channel7 selected
  *            @arg ADC_Channel_8: ADC Channel8 selected
  *            @arg ADC_Channel_9: ADC Channel9 selected
  *            @arg ADC_Channel_10: ADC Channel10 selected
  *            @arg ADC_Channel_11: ADC Channel11 selected
  *            @arg ADC_Channel_12: ADC Channel12 selected
  *            @arg ADC_Channel_13: ADC Channel13 selected
  *            @arg ADC_Channel_14: ADC Channel14 selected
  *            @arg ADC_Channel_15: ADC Channel15 selected
  *            @arg ADC_Channel_16: ADC Channel16 selected
  *            @arg ADC_Channel_17: ADC Channel17 selected
  *            @arg ADC_Channel_18: ADC Channel18 selected
  * @retval None
  */
static inline void adc_analog_watchdog_single_channel_config(ADC_TypeDef* ADCx, uint8_t ADC_Channel)
{
  uint32_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = ADCx->CR1;

  /* Clear the Analog watchdog channel select bits */
  tmpreg &= CR1_AWDCH_RESET;

  /* Set the Analog watchdog channel */
  tmpreg |= ADC_Channel;

  /* Store the new register value */
  ADCx->CR1 = tmpreg;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the temperature sensor and Vrefint channels.
  * @param  NewState: new state of the temperature sensor and Vrefint channels.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_temp_sensor_vref_int_cmd( bool enable )
{
  if ( enable )
  {
    /* Enable the temperature sensor and Vrefint channel*/
    ADC->CCR |= (uint32_t)ADC_CCR_TSVREFE;
  }
  else
  {
    /* Disable the temperature sensor and Vrefint channel*/
    ADC->CCR &= (uint32_t)(~ADC_CCR_TSVREFE);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the VBAT (Voltage Battery) channel.
  * @param  NewState: new state of the VBAT channel.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_vbat_cmd(bool enable)
{
  if ( enable )
  {
    /* Enable the VBAT channel*/
    ADC->CCR |= (uint32_t)ADC_CCR_VBATE;
  }
  else
  {
    /* Disable the VBAT channel*/
    ADC->CCR &= (uint32_t)(~ADC_CCR_VBATE);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures for the selected ADC regular channel its corresponding
  *         rank in the sequencer and its sample time.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure.
  *          This parameter can be one of the following values:
  *            @arg ADC_Channel_0: ADC Channel0 selected
  *            @arg ADC_Channel_1: ADC Channel1 selected
  *            @arg ADC_Channel_2: ADC Channel2 selected
  *            @arg ADC_Channel_3: ADC Channel3 selected
  *            @arg ADC_Channel_4: ADC Channel4 selected
  *            @arg ADC_Channel_5: ADC Channel5 selected
  *            @arg ADC_Channel_6: ADC Channel6 selected
  *            @arg ADC_Channel_7: ADC Channel7 selected
  *            @arg ADC_Channel_8: ADC Channel8 selected
  *            @arg ADC_Channel_9: ADC Channel9 selected
  *            @arg ADC_Channel_10: ADC Channel10 selected
  *            @arg ADC_Channel_11: ADC Channel11 selected
  *            @arg ADC_Channel_12: ADC Channel12 selected
  *            @arg ADC_Channel_13: ADC Channel13 selected
  *            @arg ADC_Channel_14: ADC Channel14 selected
  *            @arg ADC_Channel_15: ADC Channel15 selected
  *            @arg ADC_Channel_16: ADC Channel16 selected
  *            @arg ADC_Channel_17: ADC Channel17 selected
  *            @arg ADC_Channel_18: ADC Channel18 selected
  * @param  Rank: The rank in the regular group sequencer.
  *          This parameter must be between 1 to 16.
  * @param  ADC_SampleTime: The sample time value to be set for the selected channel.
  *          This parameter can be one of the following values:
  *            @arg ADC_SampleTime_3Cycles: Sample time equal to 3 cycles
  *            @arg ADC_SampleTime_15Cycles: Sample time equal to 15 cycles
  *            @arg ADC_SampleTime_28Cycles: Sample time equal to 28 cycles
  *            @arg ADC_SampleTime_56Cycles: Sample time equal to 56 cycles
  *            @arg ADC_SampleTime_84Cycles: Sample time equal to 84 cycles
  *            @arg ADC_SampleTime_112Cycles: Sample time equal to 112 cycles
  *            @arg ADC_SampleTime_144Cycles: Sample time equal to 144 cycles
  *            @arg ADC_SampleTime_480Cycles: Sample time equal to 480 cycles
  * @retval None
  */
static inline void adc_regular_channel_config(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime)
{
  uint32_t tmpreg1 = 0, tmpreg2 = 0;

  /* if ADC_Channel_10 ... ADC_Channel_18 is selected */
  if (ADC_Channel > ADC_Channel_9)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR1;

    /* Calculate the mask to clear */
    tmpreg2 = SMPR1_SMP_SET << (3 * (ADC_Channel - 10));

    /* Clear the old sample time */
    tmpreg1 &= ~tmpreg2;

    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * (ADC_Channel - 10));

    /* Set the new sample time */
    tmpreg1 |= tmpreg2;

    /* Store the new register value */
    ADCx->SMPR1 = tmpreg1;
  }
  else /* ADC_Channel include in ADC_Channel_[0..9] */
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR2;

    /* Calculate the mask to clear */
    tmpreg2 = SMPR2_SMP_SET << (3 * ADC_Channel);

    /* Clear the old sample time */
    tmpreg1 &= ~tmpreg2;

    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * ADC_Channel);

    /* Set the new sample time */
    tmpreg1 |= tmpreg2;

    /* Store the new register value */
    ADCx->SMPR2 = tmpreg1;
  }
  /* For Rank 1 to 6 */
  if (Rank < 7)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR3;

    /* Calculate the mask to clear */
    tmpreg2 = SQR3_SQ_SET << (5 * (Rank - 1));

    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;

    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 1));

    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;

    /* Store the new register value */
    ADCx->SQR3 = tmpreg1;
  }
  /* For Rank 7 to 12 */
  else if (Rank < 13)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR2;

    /* Calculate the mask to clear */
    tmpreg2 = SQR2_SQ_SET << (5 * (Rank - 7));

    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;

    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 7));

    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;

    /* Store the new register value */
    ADCx->SQR2 = tmpreg1;
  }
  /* For Rank 13 to 16 */
  else
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR1;

    /* Calculate the mask to clear */
    tmpreg2 = SQR1_SQ_SET << (5 * (Rank - 13));

    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;

    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 13));

    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;

    /* Store the new register value */
    ADCx->SQR1 = tmpreg1;
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables the selected ADC software start conversion of the regular channels.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval None
  */
static inline void adc_software_start_conv(ADC_TypeDef* ADCx)
{
  /* Enable the selected ADC conversion for regular group */
  ADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Gets the selected ADC Software start regular conversion Status.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval The new state of ADC software start conversion (SET or RESET).
  */
static inline bool adc_get_software_start_conv_status(ADC_TypeDef* ADCx)
{

  /* Check the status of SWSTART bit */
  return ((ADCx->CR2 & ADC_CR2_JSWSTART) != (uint32_t)0);

}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the EOC on each regular channel conversion
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC EOC flag rising
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_eoc_on_each_regular_channel_cmd(ADC_TypeDef* ADCx, bool enable)
{
  if ( enable )
  {
    /* Enable the selected ADC EOC rising on each regular channel conversion */
    ADCx->CR2 |= (uint32_t)ADC_CR2_EOCS;
  }
  else
  {
    /* Disable the selected ADC EOC rising on each regular channel conversion */
    ADCx->CR2 &= (uint32_t)(~ADC_CR2_EOCS);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the ADC continuous conversion mode
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC continuous conversion mode
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_continuous_mode_cmd(ADC_TypeDef* ADCx, bool enable )
{

  if ( enable )
  {
    /* Enable the selected ADC continuous conversion mode */
    ADCx->CR2 |= (uint32_t)ADC_CR2_CONT;
  }
  else
  {
    /* Disable the selected ADC continuous conversion mode */
    ADCx->CR2 &= (uint32_t)(~ADC_CR2_CONT);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures the discontinuous mode for the selected ADC regular group
  *         channel.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  Number: specifies the discontinuous mode regular channel count value.
  *          This number must be between 1 and 8.
  * @retval None
  */
static inline void adc_disc_mode_channel_count_config(ADC_TypeDef* ADCx, uint8_t Number)
{
  uint32_t tmpreg1 = 0;
  uint32_t tmpreg2 = 0;

  /* Get the old register value */
  tmpreg1 = ADCx->CR1;

  /* Clear the old discontinuous mode channel count */
  tmpreg1 &= CR1_DISCNUM_RESET;

  /* Set the discontinuous mode channel count */
  tmpreg2 = Number - 1;
  tmpreg1 |= tmpreg2 << 13;

  /* Store the new register value */
  ADCx->CR1 = tmpreg1;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the discontinuous mode on regular group channel
  *         for the specified ADC
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC discontinuous mode on
  *         regular group channel.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_disc_mode_cmd(ADC_TypeDef* ADCx, bool enable)
{

  if ( enable )
  {
    /* Enable the selected ADC regular discontinuous mode */
    ADCx->CR1 |= (uint32_t)ADC_CR1_DISCEN;
  }
  else
  {
    /* Disable the selected ADC regular discontinuous mode */
    ADCx->CR1 &= (uint32_t)(~ADC_CR1_DISCEN);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Returns the last ADCx conversion result data for regular channel.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval The Data conversion value.
  */
static inline uint16_t adc_get_conversion_value(ADC_TypeDef* ADCx)
{
  /* Return the selected ADC conversion value */
  return (uint16_t) ADCx->DR;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Returns the last ADC1, ADC2 and ADC3 regular conversions results
  *         data in the selected multi mode.
  * @param  None
  * @retval The Data conversion value.
  * @note   In dual mode, the value returned by this function is as following
  *           Data[15:0] : these bits contain the regular data of ADC1.
  *           Data[31:16]: these bits contain the regular data of ADC2.
  * @note   In triple mode, the value returned by this function is as following
  *           Data[15:0] : these bits contain alternatively the regular data of ADC1, ADC3 and ADC2.
  *           Data[31:16]: these bits contain alternatively the regular data of ADC2, ADC1 and ADC3.
  */
static inline uint32_t adc_get_multi_mode_conversion_value(void)
{
  /* Return the multi mode conversion value */
  return (*(__IO uint32_t *) CDR_ADDRESS);
}
/* ---------------------------------------------------------------------------- */
/**
 * @brief  Enables or disables the specified ADC DMA request.
 * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
 * @param  NewState: new state of the selected ADC DMA transfer.
 *          This parameter can be: ENABLE or DISABLE.
 * @retval None
 */
static inline void adc_dma_cmd(ADC_TypeDef* ADCx, bool enable)
{

 if ( enable )
 {
   /* Enable the selected ADC DMA request */
   ADCx->CR2 |= (uint32_t)ADC_CR2_DMA;
 }
 else
 {
   /* Disable the selected ADC DMA request */
   ADCx->CR2 &= (uint32_t)(~ADC_CR2_DMA);
 }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the ADC DMA request after last transfer (Single-ADC mode)
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC DMA request after last transfer.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_dma_request_after_last_transfer_cmd(ADC_TypeDef* ADCx, bool enable )
{
  if (enable)
  {
    /* Enable the selected ADC DMA request after last transfer */
    ADCx->CR2 |= (uint32_t)ADC_CR2_DDS;
  }
  else
  {
    /* Disable the selected ADC DMA request after last transfer */
    ADCx->CR2 &= (uint32_t)(~ADC_CR2_DDS);
  }
}

/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the ADC DMA request after last transfer in multi ADC mode
  * @param  NewState: new state of the selected ADC DMA request after last transfer.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   if Enabled, DMA requests are issued as long as data are converted and
  *         DMA mode for multi ADC mode (selected using ADC_CommonInit() function
  *         by ADC_CommonInitStruct.ADC_DMAAccessMode structure member) is
  *          ADC_DMAAccessMode_1, ADC_DMAAccessMode_2 or ADC_DMAAccessMode_3.
  * @retval None
  */
static inline void adc_multi_mode_dma_request_after_last_transfer_cmd(bool enable)
{
  if ( enable )
  {
    /* Enable the selected ADC DMA request after last transfer */
    ADC->CCR |= (uint32_t)ADC_CCR_DDS;
  }
  else
  {
    /* Disable the selected ADC DMA request after last transfer */
    ADC->CCR &= (uint32_t)(~ADC_CCR_DDS);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures for the selected ADC injected channel its corresponding
  *         rank in the sequencer and its sample time.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure.
  *          This parameter can be one of the following values:
  *            @arg ADC_Channel_0: ADC Channel0 selected
  *            @arg ADC_Channel_1: ADC Channel1 selected
  *            @arg ADC_Channel_2: ADC Channel2 selected
  *            @arg ADC_Channel_3: ADC Channel3 selected
  *            @arg ADC_Channel_4: ADC Channel4 selected
  *            @arg ADC_Channel_5: ADC Channel5 selected
  *            @arg ADC_Channel_6: ADC Channel6 selected
  *            @arg ADC_Channel_7: ADC Channel7 selected
  *            @arg ADC_Channel_8: ADC Channel8 selected
  *            @arg ADC_Channel_9: ADC Channel9 selected
  *            @arg ADC_Channel_10: ADC Channel10 selected
  *            @arg ADC_Channel_11: ADC Channel11 selected
  *            @arg ADC_Channel_12: ADC Channel12 selected
  *            @arg ADC_Channel_13: ADC Channel13 selected
  *            @arg ADC_Channel_14: ADC Channel14 selected
  *            @arg ADC_Channel_15: ADC Channel15 selected
  *            @arg ADC_Channel_16: ADC Channel16 selected
  *            @arg ADC_Channel_17: ADC Channel17 selected
  *            @arg ADC_Channel_18: ADC Channel18 selected
  * @param  Rank: The rank in the injected group sequencer.
  *          This parameter must be between 1 to 4.
  * @param  ADC_SampleTime: The sample time value to be set for the selected channel.
  *          This parameter can be one of the following values:
  *            @arg ADC_SampleTime_3Cycles: Sample time equal to 3 cycles
  *            @arg ADC_SampleTime_15Cycles: Sample time equal to 15 cycles
  *            @arg ADC_SampleTime_28Cycles: Sample time equal to 28 cycles
  *            @arg ADC_SampleTime_56Cycles: Sample time equal to 56 cycles
  *            @arg ADC_SampleTime_84Cycles: Sample time equal to 84 cycles
  *            @arg ADC_SampleTime_112Cycles: Sample time equal to 112 cycles
  *            @arg ADC_SampleTime_144Cycles: Sample time equal to 144 cycles
  *            @arg ADC_SampleTime_480Cycles: Sample time equal to 480 cycles
  * @retval None
  */
static inline void adc_injected_channel_config(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime)
{
  uint32_t tmpreg1 = 0, tmpreg2 = 0, tmpreg3 = 0;

  /* if ADC_Channel_10 ... ADC_Channel_18 is selected */
  if (ADC_Channel > ADC_Channel_9)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR1;
    /* Calculate the mask to clear */
    tmpreg2 = SMPR1_SMP_SET << (3*(ADC_Channel - 10));
    /* Clear the old sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3*(ADC_Channel - 10));
    /* Set the new sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR1 = tmpreg1;
  }
  else /* ADC_Channel include in ADC_Channel_[0..9] */
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR2;
    /* Calculate the mask to clear */
    tmpreg2 = SMPR2_SMP_SET << (3 * ADC_Channel);
    /* Clear the old sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * ADC_Channel);
    /* Set the new sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR2 = tmpreg1;
  }
  /* Rank configuration */
  /* Get the old register value */
  tmpreg1 = ADCx->JSQR;
  /* Get JL value: Number = JL+1 */
  tmpreg3 =  (tmpreg1 & JSQR_JL_SET)>> 20;
  /* Calculate the mask to clear: ((Rank-1)+(4-JL-1)) */
  tmpreg2 = JSQR_JSQ_SET << (5 * (uint8_t)((Rank + 3) - (tmpreg3 + 1)));
  /* Clear the old JSQx bits for the selected rank */
  tmpreg1 &= ~tmpreg2;
  /* Calculate the mask to set: ((Rank-1)+(4-JL-1)) */
  tmpreg2 = (uint32_t)ADC_Channel << (5 * (uint8_t)((Rank + 3) - (tmpreg3 + 1)));
  /* Set the JSQx bits for the selected rank */
  tmpreg1 |= tmpreg2;
  /* Store the new register value */
  ADCx->JSQR = tmpreg1;
}
/* ---------------------------------------------------------------------------- */

/**
  * @brief  Configures the sequencer length for injected channels
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  Length: The sequencer length.
  *          This parameter must be a number between 1 to 4.
  * @retval None
  */
static inline void adc_injected_sequencer_length_config(ADC_TypeDef* ADCx, uint8_t Length)
{
  uint32_t tmpreg1 = 0;
  uint32_t tmpreg2 = 0;

  /* Get the old register value */
  tmpreg1 = ADCx->JSQR;

  /* Clear the old injected sequence length JL bits */
  tmpreg1 &= JSQR_JL_RESET;

  /* Set the injected sequence length JL bits */
  tmpreg2 = Length - 1;
  tmpreg1 |= tmpreg2 << 20;

  /* Store the new register value */
  ADCx->JSQR = tmpreg1;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Set the injected channels conversion value offset
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_InjectedChannel: the ADC injected channel to set its offset.
  *          This parameter can be one of the following values:
  *            @arg ADC_InjectedChannel_1: Injected Channel1 selected
  *            @arg ADC_InjectedChannel_2: Injected Channel2 selected
  *            @arg ADC_InjectedChannel_3: Injected Channel3 selected
  *            @arg ADC_InjectedChannel_4: Injected Channel4 selected
  * @param  Offset: the offset value for the selected ADC injected channel
  *          This parameter must be a 12bit value.
  * @retval None
  */
static inline void adc_set_injected_offset(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel, uint16_t Offset)
{
    __IO uint32_t tmp = 0;

  tmp = (uint32_t)ADCx;
  tmp += ADC_InjectedChannel;

  /* Set the selected injected channel data offset */
 *(__IO uint32_t *) tmp = (uint32_t)Offset;
}
/* ---------------------------------------------------------------------------- */
/**
 * @brief  Configures the ADCx external trigger for injected channels conversion.
 * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
 * @param  ADC_ExternalTrigInjecConv: specifies the ADC trigger to start injected conversion.
 *          This parameter can be one of the following values:
 *            @arg ADC_ExternalTrigInjecConv_T1_CC4: Timer1 capture compare4 selected
 *            @arg ADC_ExternalTrigInjecConv_T1_TRGO: Timer1 TRGO event selected
 *            @arg ADC_ExternalTrigInjecConv_T2_CC1: Timer2 capture compare1 selected
 *            @arg ADC_ExternalTrigInjecConv_T2_TRGO: Timer2 TRGO event selected
 *            @arg ADC_ExternalTrigInjecConv_T3_CC2: Timer3 capture compare2 selected
 *            @arg ADC_ExternalTrigInjecConv_T3_CC4: Timer3 capture compare4 selected
 *            @arg ADC_ExternalTrigInjecConv_T4_CC1: Timer4 capture compare1 selected
 *            @arg ADC_ExternalTrigInjecConv_T4_CC2: Timer4 capture compare2 selected
 *            @arg ADC_ExternalTrigInjecConv_T4_CC3: Timer4 capture compare3 selected
 *            @arg ADC_ExternalTrigInjecConv_T4_TRGO: Timer4 TRGO event selected
 *            @arg ADC_ExternalTrigInjecConv_T5_CC4: Timer5 capture compare4 selected
 *            @arg ADC_ExternalTrigInjecConv_T5_TRGO: Timer5 TRGO event selected
 *            @arg ADC_ExternalTrigInjecConv_T8_CC2: Timer8 capture compare2 selected
 *            @arg ADC_ExternalTrigInjecConv_T8_CC3: Timer8 capture compare3 selected
 *            @arg ADC_ExternalTrigInjecConv_T8_CC4: Timer8 capture compare4 selected
 *            @arg ADC_ExternalTrigInjecConv_Ext_IT15: External interrupt line 15 event selected
 * @retval None
 */
static inline void adc_external_trig_injected_conv_config(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConv)
{
 uint32_t tmpreg = 0;

 /* Get the old register value */
 tmpreg = ADCx->CR2;

 /* Clear the old external event selection for injected group */
 tmpreg &= CR2_JEXTSEL_RESET;

 /* Set the external event selection for injected group */
 tmpreg |= ADC_ExternalTrigInjecConv;

 /* Store the new register value */
 ADCx->CR2 = tmpreg;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures the ADCx external trigger edge for injected channels conversion.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_ExternalTrigInjecConvEdge: specifies the ADC external trigger edge
  *         to start injected conversion.
  *          This parameter can be one of the following values:
  *            @arg ADC_ExternalTrigInjecConvEdge_None: external trigger disabled for
  *                                                     injected conversion
  *            @arg ADC_ExternalTrigInjecConvEdge_Rising: detection on rising edge
  *            @arg ADC_ExternalTrigInjecConvEdge_Falling: detection on falling edge
  *            @arg ADC_ExternalTrigInjecConvEdge_RisingFalling: detection on both rising
  *                                                               and falling edge
  * @retval None
  */
static inline void adc_external_trig_injected_conv_edge_config(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConvEdge)
{
  uint32_t tmpreg = 0;

  /* Get the old register value */
  tmpreg = ADCx->CR2;
  /* Clear the old external trigger edge for injected group */
  tmpreg &= CR2_JEXTEN_RESET;
  /* Set the new external trigger edge for injected group */
  tmpreg |= ADC_ExternalTrigInjecConvEdge;
  /* Store the new register value */
  ADCx->CR2 = tmpreg;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables the selected ADC software start conversion of the injected channels.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval None
  */
static inline void adc_software_start_injected_conv(ADC_TypeDef* ADCx)
{
  /* Enable the selected ADC conversion for injected group */
  ADCx->CR2 |= (uint32_t)ADC_CR2_JSWSTART;
}
/* ---------------------------------------------------------------------------- */

/**
  * @brief  Gets the selected ADC Software start injected conversion Status.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval The new state of ADC software start injected conversion (SET or RESET).
  */
static inline bool ADC_GetSoftwareStartInjectedConvCmdStatus(ADC_TypeDef* ADCx)
{
    /* Check the status of JSWSTART bit */
  return ((ADCx->CR2 & ADC_CR2_JSWSTART) != (uint32_t)0);
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the selected ADC automatic injected group
  *         conversion after regular one.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC auto injected conversion
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_auto_injected_conv_cmd(ADC_TypeDef* ADCx, bool enable)
{
  if ( enable )
  {
    /* Enable the selected ADC automatic injected group conversion */
    ADCx->CR1 |= (uint32_t)ADC_CR1_JAUTO;
  }
  else
  {
    /* Disable the selected ADC automatic injected group conversion */
    ADCx->CR1 &= (uint32_t)(~ADC_CR1_JAUTO);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the discontinuous mode for injected group
  *         channel for the specified ADC
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC discontinuous mode on injected
  *         group channel.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_injected_disc_mode_cmd(ADC_TypeDef* ADCx, bool enable )
{
  if ( enable )
  {
    /* Enable the selected ADC injected discontinuous mode */
    ADCx->CR1 |= (uint32_t)ADC_CR1_JDISCEN;
  }
  else
  {
    /* Disable the selected ADC injected discontinuous mode */
    ADCx->CR1 &= (uint32_t)(~ADC_CR1_JDISCEN);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Returns the ADC injected channel conversion result
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_InjectedChannel: the converted ADC injected channel.
  *          This parameter can be one of the following values:
  *            @arg ADC_InjectedChannel_1: Injected Channel1 selected
  *            @arg ADC_InjectedChannel_2: Injected Channel2 selected
  *            @arg ADC_InjectedChannel_3: Injected Channel3 selected
  *            @arg ADC_InjectedChannel_4: Injected Channel4 selected
  * @retval The Data conversion value.
  */
static inline uint16_t adc_get_injected_conversion_value(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel)
{
  __IO uint32_t tmp = 0;

  tmp = (uint32_t)ADCx;
  tmp += ADC_InjectedChannel + JDR_OFFSET;

  /* Returns the selected injected channel conversion data value */
  return (uint16_t) (*(__IO uint32_t*)  tmp);
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the specified ADC interrupts.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt sources to be enabled or disabled.
  *          This parameter can be one of the following values:
  *            @arg ADC_IT_EOC: End of conversion interrupt mask
  *            @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *            @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  *            @arg ADC_IT_OVR: Overrun interrupt enable
  * @param  NewState: new state of the specified ADC interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void adc_it_config(ADC_TypeDef* ADCx, uint16_t ADC_IT, bool enable)
{
  uint32_t itmask = 0;

  /* Get the ADC IT index */
  itmask = (uint8_t)ADC_IT;
  itmask = (uint32_t)0x01 << itmask;

  if ( enable )
  {
    /* Enable the selected ADC interrupts */
    ADCx->CR1 |= itmask;
  }
  else
  {
    /* Disable the selected ADC interrupts */
    ADCx->CR1 &= (~(uint32_t)itmask);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the specified ADC flag is set or not.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg ADC_FLAG_AWD: Analog watchdog flag
  *            @arg ADC_FLAG_EOC: End of conversion flag
  *            @arg ADC_FLAG_JEOC: End of injected group conversion flag
  *            @arg ADC_FLAG_JSTRT: Start of injected group conversion flag
  *            @arg ADC_FLAG_STRT: Start of regular group conversion flag
  *            @arg ADC_FLAG_OVR: Overrun flag
  * @retval The new state of ADC_FLAG (SET or RESET).
  */
static inline bool adc_get_flag_status(ADC_TypeDef* ADCx, uint8_t ADC_FLAG)
{

  /* Check the status of the specified ADC flag */
  return  ((ADCx->SR & ADC_FLAG) != (uint8_t)0);
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Clears the ADCx's pending flags.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg ADC_FLAG_AWD: Analog watchdog flag
  *            @arg ADC_FLAG_EOC: End of conversion flag
  *            @arg ADC_FLAG_JEOC: End of injected group conversion flag
  *            @arg ADC_FLAG_JSTRT: Start of injected group conversion flag
  *            @arg ADC_FLAG_STRT: Start of regular group conversion flag
  *            @arg ADC_FLAG_OVR: Overrun flag
  * @retval None
  */
static inline void adc_clear_flag(ADC_TypeDef* ADCx, uint8_t ADC_FLAG)
{
  /* Clear the selected ADC flags */
  ADCx->SR = ~(uint32_t)ADC_FLAG;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the specified ADC interrupt has occurred or not.
  * @param  ADCx:   where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg ADC_IT_EOC: End of conversion interrupt mask
  *            @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *            @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  *            @arg ADC_IT_OVR: Overrun interrupt mask
  * @retval The new state of ADC_IT (SET or RESET).
  */
static inline bool adc_get_it_status(ADC_TypeDef* ADCx, uint16_t ADC_IT)
{
  uint32_t itmask = 0, enablestatus = 0;

  /* Get the ADC IT index */
  itmask = ADC_IT >> 8;

  /* Get the ADC_IT enable bit status */
  enablestatus = (ADCx->CR1 & ((uint32_t)0x01 << (uint8_t)ADC_IT)) ;

  /* Check the status of the specified ADC interrupt */
  return (((ADCx->SR & itmask) != (uint32_t)0) && enablestatus);
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Clears the ADCx's interrupt pending bits.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt pending bit to clear.
  *          This parameter can be one of the following values:
  *            @arg ADC_IT_EOC: End of conversion interrupt mask
  *            @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *            @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  *            @arg ADC_IT_OVR: Overrun interrupt mask
  * @retval None
  */
static inline void adc_clear_it_pending_bit(ADC_TypeDef* ADCx, uint16_t ADC_IT)
{
  uint8_t itmask = 0;
  /* Get the ADC IT index */
  itmask = (uint8_t)(ADC_IT >> 8);
  /* Clear the selected ADC interrupt pending bits */
  ADCx->SR = ~(uint32_t)itmask;
}

/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

 /* ADC DISCNUM mask */
 #undef CR1_DISCNUM_RESET
 #undef CR1_AWDCH_RESET
 #undef CR1_AWDMode_RESET
 #undef CR1_CLEAR_MASK
 #undef CR2_EXTEN_RESET
 #undef CR2_JEXTEN_RESET
 #undef CR2_JEXTSEL_RESET
 #undef CR2_CLEAR_MASK
 #undef SQR3_SQ_SET
 #undef SQR2_SQ_SET
 #undef SQR1_SQ_SET
 #undef SQR1_L_RESET
 #undef JSQR_JSQ_SET
 #undef JSQR_JL_SET
 #undef JSQR_JL_RESET
 #undef SMPR1_SMP_SET
 #undef SMPR2_SMP_SET
 #undef JDR_OFFSET
 #undef CDR_ADDRESS
 #undef CR_CLEAR_MASK

#endif /* STM32ADC_H_ */
