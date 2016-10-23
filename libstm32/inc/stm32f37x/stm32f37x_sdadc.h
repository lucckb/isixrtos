#pragma once

/* The SDADC channels are defined as follow:
   - in 16-bit LSB the channel mask is set
   - in 16-bit MSB the channel number is set 
   e.g. for channel 5 definition:  
        - the channel mask is 0x00000020 (bit 5 is set) 
        - the channel number 5 is 0x00050000 
        --> Consequently, channel 5 definition is 0x00000020 | 0x00050000 = 0x00050020 */
#define SDADC_Channel_0                              ((uint32_t)0x00000001)
#define SDADC_Channel_1                              ((uint32_t)0x00010002)
#define SDADC_Channel_2                              ((uint32_t)0x00020004)
#define SDADC_Channel_3                              ((uint32_t)0x00030008)
#define SDADC_Channel_4                              ((uint32_t)0x00040010)
#define SDADC_Channel_5                              ((uint32_t)0x00050020)
#define SDADC_Channel_6                              ((uint32_t)0x00060040)
#define SDADC_Channel_7                              ((uint32_t)0x00070080)
#define SDADC_Channel_8                              ((uint32_t)0x00080100)


/** @defgroup SDADC_Conf 
  * @{
  */
  
#define SDADC_Conf_0                              ((uint32_t)0x00000000) /*!< Configuration 0 selected */
#define SDADC_Conf_1                              ((uint32_t)0x00000001) /*!< Configuration 1 selected */
#define SDADC_Conf_2                              ((uint32_t)0x00000002) /*!< Configuration 2 selected */


/** @defgroup SDADC_InputMode
  * @{
  */

#define SDADC_InputMode_Diff                      ((uint32_t)0x00000000) /*!< Conversions are executed in differential mode */
#define SDADC_InputMode_SEOffset                  SDADC_CONF0R_SE0_0     /*!< Conversions are executed in single ended offset mode */
#define SDADC_InputMode_SEZeroReference           SDADC_CONF0R_SE0       /*!< Conversions are executed in single ended zero-volt reference mode */


/** @defgroup SDADC_Gain 
  * @{
  */

#define SDADC_Gain_1                              ((uint32_t)0x00000000)  /*!< Gain equal to 1 */
#define SDADC_Gain_2                              SDADC_CONF0R_GAIN0_0    /*!< Gain equal to 2 */
#define SDADC_Gain_4                              SDADC_CONF0R_GAIN0_1    /*!< Gain equal to 4 */
#define SDADC_Gain_8                              ((uint32_t)0x00300000)  /*!< Gain equal to 8 */
#define SDADC_Gain_16                             SDADC_CONF0R_GAIN0_2    /*!< Gain equal to 16 */
#define SDADC_Gain_32                             ((uint32_t)0x00500000)  /*!< Gain equal to 32 */
#define SDADC_Gain_1_2                            SDADC_CONF0R_GAIN0      /*!< Gain equal to 1/2 */


#define SDADC_CommonMode_VSSA                      ((uint32_t)0x00000000) /*!< Select SDADC VSSA as common mode */
#define SDADC_CommonMode_VDDA_2                    SDADC_CONF0R_COMMON0_0 /*!< Select SDADC VDDA/2 as common mode */
#define SDADC_CommonMode_VDDA                      SDADC_CONF0R_COMMON0_1 /*!< Select SDADC VDDA as common mode */



/** @defgroup SDADC_ExternalTrigger_sources
  * @{
  */
#define SDADC_ExternalTrigInjecConv_T13_CC1               ((uint32_t)0x00000000) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T14_CC1               ((uint32_t)0x00000100) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T16_CC1               ((uint32_t)0x00000000) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T17_CC1               ((uint32_t)0x00000000) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T12_CC1               ((uint32_t)0x00000100) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T12_CC2               ((uint32_t)0x00000100) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T15_CC2               ((uint32_t)0x00000200) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T2_CC3                ((uint32_t)0x00000200) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T2_CC4                ((uint32_t)0x00000200) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T3_CC1                ((uint32_t)0x00000300) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T3_CC2                ((uint32_t)0x00000300) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T3_CC3                ((uint32_t)0x00000300) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T4_CC1                ((uint32_t)0x00000400) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T4_CC2                ((uint32_t)0x00000400) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T4_CC3                ((uint32_t)0x00000400) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T19_CC2               ((uint32_t)0x00000500) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T19_CC3               ((uint32_t)0x00000500) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T19_CC4               ((uint32_t)0x00000500) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_Ext_IT11              ((uint32_t)0x00000700) /*!< Trigger source for SDADC1, SDADC2 and SDADC3 */
#define SDADC_ExternalTrigInjecConv_Ext_IT15              ((uint32_t)0x00000600) /*!< Trigger source for SDADC1, SDADC2 and SDADC3 */

/** @defgroup SDADC_external_trigger_edge_for_injected_channels_conversion 
  * @{
  */ 
#define SDADC_ExternalTrigInjecConvEdge_None               ((uint32_t) 0x00000000)
#define SDADC_ExternalTrigInjecConvEdge_Rising             SDADC_CR2_JEXTEN_0
#define SDADC_ExternalTrigInjecConvEdge_Falling            SDADC_CR2_JEXTEN_1
#define SDADC_ExternalTrigInjecConvEdge_RisingFalling      SDADC_CR2_JEXTEN


/** @defgroup SDADC_DMATransfer_modes 
  * @{
  */ 
#define SDADC_DMATransfer_Regular                SDADC_CR1_RDMAEN          /*!< DMA requests enabled for regular conversions */
#define SDADC_DMATransfer_Injected               SDADC_CR1_JDMAEN          /*!< DMA requests enabled for injected conversions */


/** @defgroup SDADC_CalibrationSequence 
  * @{
  */ 
#define SDADC_CalibrationSequence_1                   ((uint32_t)0x00000000) /*!< One calibration sequence to calculate offset of conf0 (OFFSET0[11:0]) */
#define SDADC_CalibrationSequence_2                   SDADC_CR2_CALIBCNT_0   /*!< Two calibration sequences to calculate offset of conf0 and conf1 (OFFSET0[11:0] and OFFSET1[11:0]) */
#define SDADC_CalibrationSequence_3                   SDADC_CR2_CALIBCNT_1   /*!< Three calibration sequences to calculate offset of conf0, conf1 and conf2 (OFFSET0[11:0], OFFSET1[11:0], and OFFSET2[11:0]) */


/** @defgroup SDADC_VREF
  * @{
  */

#define SDADC_VREF_Ext                            ((uint32_t)0x00000000) /*!< The reference voltage is forced externally using VREF pin */
#define SDADC_VREF_VREFINT1                       SDADC_CR1_REFV_0       /*!< The reference voltage is forced internally to 1.22V VREFINT */
#define SDADC_VREF_VREFINT2                       SDADC_CR1_REFV_1       /*!< The reference voltage is forced internally to 1.8V VREFINT */
#define SDADC_VREF_VDDA                           SDADC_CR1_REFV         /*!< The reference voltage is forced internally to VDDA */


/** @defgroup SDADC_interrupts_definition
  * @{
  */

#define SDADC_IT_EOCAL                               ((uint32_t)0x00000001) /*!< End of calibration flag */
#define SDADC_IT_JEOC                                ((uint32_t)0x00000002) /*!< End of injected conversion flag */
#define SDADC_IT_JOVR                                ((uint32_t)0x00000004) /*!< Injected conversion overrun flag */
#define SDADC_IT_REOC                                ((uint32_t)0x00000008) /*!< End of regular conversion flag */
#define SDADC_IT_ROVR                                ((uint32_t)0x00000010) /*!< Regular conversion overrun flag */


/** @defgroup SDADC_flags_definition
  * @{
  */

#define SDADC_FLAG_EOCAL                             ((uint32_t)0x00000001) /*!< End of calibration flag */
#define SDADC_FLAG_JEOC                              ((uint32_t)0x00000002) /*!< End of injected conversion flag */
#define SDADC_FLAG_JOVR                              ((uint32_t)0x00000004) /*!< Injected conversion overrun flag */
#define SDADC_FLAG_REOC                              ((uint32_t)0x00000008) /*!< End of regular conversion flag */
#define SDADC_FLAG_ROVR                              ((uint32_t)0x00000010) /*!< Regular conversion overrun flag */
#define SDADC_FLAG_CALIBIP                           ((uint32_t)0x00001000) /*!< Calibration in progress status */
#define SDADC_FLAG_JCIP                              ((uint32_t)0x00002000) /*!< Injected conversion in progress status */
#define SDADC_FLAG_RCIP                              ((uint32_t)0x00004000) /*!< Regular conversion in progress status */
#define SDADC_FLAG_STABIP                            ((uint32_t)0x00008000) /*!< Stabilization in progress status */
#define SDADC_FLAG_INITRDY                           ((uint32_t)0x80000000) /*!< Initialization mode is ready */

