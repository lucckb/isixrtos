#pragma once

/** @defgroup COMP_InvertingInput
  * @{
  */

#define COMP_InvertingInput_1_4VREFINT          ((uint32_t)0x00000000) /*!< 1/4 VREFINT connected to comparator inverting input */
#define COMP_InvertingInput_1_2VREFINT          COMP_CSR_COMP1INSEL_0  /*!< 1/2 VREFINT connected to comparator inverting input */
#define COMP_InvertingInput_3_4VREFINT          COMP_CSR_COMP1INSEL_1  /*!< 3/4 VREFINT connected to comparator inverting input */
#define COMP_InvertingInput_VREFINT             ((uint32_t)0x00000030) /*!< VREFINT connected to comparator inverting input */
#define COMP_InvertingInput_DAC1OUT1            COMP_CSR_COMP1INSEL_2  /*!< DAC1_OUT1 (PA4) connected to comparator inverting input */
#define COMP_InvertingInput_DAC1OUT2            ((uint32_t)0x00000050) /*!< DAC1_OUT2 (PA5) connected to comparator inverting input */
#define COMP_InvertingInput_DAC2OUT1            ((uint32_t)0x00000070) /*!< DAC2_OUT1 (PA6) connected to comparator inverting input */
#define COMP_InvertingInput_IO                  ((uint32_t)0x00000060) /*!< I/O (PA0 for COMP1 and PA2 for COMP2) connected to comparator inverting input */

/** @defgroup COMP_Output
  * @{
  */
#define COMP_Output_None                  ((uint32_t)0x00000000) /*!< COMP output isn't connected to other peripherals */
#define COMP_Output_TIM5IC4               ((uint32_t)0x00000600) /*!< COMP output connected to TIM5 Input Capture 4 */
#define COMP_Output_TIM4IC1               ((uint32_t)0x00000200) /*!< COMP output connected to TIM4 Input Capture 1 */
#define COMP_Output_TIM5OCREFCLR          COMP_CSR_COMP1OUTSEL   /*!< COMP output connected to TIM5 OCREF Clear */
#define COMP_Output_TIM4OCREFCLR          ((uint32_t)0x00000300) /*!< COMP output connected to TIM4 OCREF Clear */
#define COMP_Output_TIM15BKIN             COMP_CSR_COMP1OUTSEL_0 /*!< COMP output connected to TIM15 Break Input (BKIN) */
#define COMP_Output_TIM16BKIN             COMP_CSR_COMP1OUTSEL_0 /*!< COMP output connected to TIM16 Break Input (BKIN) */
#define COMP_Output_TIM2IC4               COMP_CSR_COMP1OUTSEL_2 /*!< COMP output connected to TIM2 Input Capture 4 */
#define COMP_Output_TIM2OCREFCLR          ((uint32_t)0x00000500) /*!< COMP output connected to TIM2 OCREF Clear */
#define COMP_Output_TIM3IC1               ((uint32_t)0x00000600) /*!< COMP output connected to TIM3 Input Capture 1 */
#define COMP_Output_TIM3OCREFCLR          COMP_CSR_COMP1OUTSEL   /*!< COMP output connected to TIM3 OCREF Clear */


/** @defgroup COMP_OutputPolarity
  * @{
  */
#define COMP_OutputPol_NonInverted          ((uint32_t)0x00000000)  /*!< COMP output on GPIO isn't inverted */
#define COMP_OutputPol_Inverted             COMP_CSR_COMP1POL       /*!< COMP output on GPIO is inverted */


/** @defgroup COMP_Hysteresis
  * @{
  */
/* Please refer to the electrical characteristics in the device datasheet for
   the hysteresis level */
#define COMP_Hysteresis_No                         0x00000000           /*!< No hysteresis */
#define COMP_Hysteresis_Low                        COMP_CSR_COMP1HYST_0 /*!< Hysteresis level low */
#define COMP_Hysteresis_Medium                     COMP_CSR_COMP1HYST_1 /*!< Hysteresis level medium */
#define COMP_Hysteresis_High                       COMP_CSR_COMP1HYST   /*!< Hysteresis level high */



/** @defgroup COMP_Mode
  * @{
  */
/* Please refer to the electrical characteristics in the device datasheet for
   the power consumption values */
#define COMP_Mode_HighSpeed                     0x00000000           /*!< High Speed */
#define COMP_Mode_MediumSpeed                   COMP_CSR_COMP1MODE_0 /*!< Medium Speed */
#define COMP_Mode_LowPower                      COMP_CSR_COMP1MODE_1 /*!< Low power mode */
#define COMP_Mode_UltraLowPower                 COMP_CSR_COMP1MODE   /*!< Ultra-low power mode */


/** @defgroup COMP_OutputLevel
  * @{
  */ 
/* When output polarity is not inverted, comparator output is high when
   the non-inverting input is at a higher voltage than the inverting input */
#define COMP_OutputLevel_High                   COMP_CSR_COMP1OUT
/* When output polarity is not inverted, comparator output is low when
   the non-inverting input is at a lower voltage than the inverting input*/
#define COMP_OutputLevel_Low                    ((uint32_t)0x00000000)


