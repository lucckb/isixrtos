/*
 * 	stm32adc.h
 *	STM32 adc converter for F4 platform
 *  Created on: 23-07-2012
 *      Author: lucck
 */

#ifndef STM32F4ADC_H_
#define STM32F4ADC_H_

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
   tmpreg1 |= (uint32_t)(ADC_InitStruct->ADC_DataAlign | \
                         ADC_InitStruct->ADC_ExternalTrigConv |
                         ADC_InitStruct->ADC_ExternalTrigConvEdge | \
                         ((uint32_t)ADC_InitStruct->ADC_ContinuousConvMode << 1));

   /* Write to ADCx CR2 */
   ADCx->CR2 = tmpreg1;
   /*---------------------------- ADCx SQR1 Configuration -----------------*/
   /* Get the ADCx SQR1 value */
   tmpreg1 = ADCx->SQR1;

   /* Clear L bits */
   tmpreg1 &= SQR1_L_RESET;

   /* Configure ADCx: regular channel sequence length */
   /* Set L bits according to ADC_NbrOfConversion value */
   tmpreg2 |= (uint8_t)(ADC_InitStruct->ADC_NbrOfConversion - (uint8_t)1);
   tmpreg1 |= ((uint32_t)tmpreg2 << 20);

   /* Write to ADCx SQR1 */
   ADCx->SQR1 = tmpreg1;
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
