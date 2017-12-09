/*
 * =====================================================================================
 *
 *       Filename:  stm32f30x_adc.h
 *
 *    Description:  STM32 ADC reg defs
 *
 *        Version:  1.0
 *        Created:  09.12.2017 19:54:48
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once


/** @defgroup ADC_ContinuousConvMode 
  * @{
  */
#define ADC_ContinuousConvMode_Enable	 ((uint32_t)0x00002000)  /*!<  ADC continuous conversion mode enable */
#define ADC_ContinuousConvMode_Disable	 ((uint32_t)0x00000000)  /*!<  ADC continuous conversion mode disable */

/** @defgroup ADC_OverunMode
  * @{
  */
#define ADC_OverrunMode_Enable	 ((uint32_t)0x00001000)  /*!<  ADC Overrun Mode enable */
#define ADC_OverrunMode_Disable	 ((uint32_t)0x00000000)  /*!<  ADC Overrun Mode disable */
/** @defgroup ADC_AutoInjecMode
  * @{
  */
#define ADC_AutoInjec_Enable	 ((uint32_t)0x02000000)  /*!<  ADC Auto injected Mode enable */
#define ADC_AutoInjec_Disable	 ((uint32_t)0x00000000)  /*!<  ADC Auto injected Mode disable */

/** @defgroup ADC_resolution 
  * @{
  */ 
#define ADC_Resolution_12b                         ((uint32_t)0x00000000)  /*!<  ADC 12-bit resolution */
#define ADC_Resolution_10b                         ((uint32_t)0x00000008)  /*!<  ADC 10-bit resolution */
#define ADC_Resolution_8b                          ((uint32_t)0x00000010)  /*!<  ADC 8-bit resolution */
#define ADC_Resolution_6b                          ((uint32_t)0x00000018)  /*!<  ADC 6-bit resolution */
/** @defgroup ADC_external_trigger_edge_for_regular_channels_conversion 
  * @{
  */
#define ADC_ExternalTrigEventEdge_None            ((uint16_t)0x0000)     /*!<  ADC No external trigger for regular conversion */
#define ADC_ExternalTrigEventEdge_RisingEdge      ((uint16_t)0x0400)     /*!<  ADC external trigger rising edge for regular conversion */
#define ADC_ExternalTrigEventEdge_FallingEdge     ((uint16_t)0x0800)     /*!<  ADC ADC external trigger falling edge for regular conversion */
#define ADC_ExternalTrigEventEdge_BothEdge        ((uint16_t)0x0C00)     /*!<  ADC ADC external trigger both edges for regular conversion */

/** @defgroup ADC_external_trigger_edge_for_Injected_channels_conversion 
  * @{
  */     
#define ADC_ExternalTrigInjecEventEdge_None		     ((uint16_t)0x0000)    /*!<  ADC No external trigger for regular conversion */
#define ADC_ExternalTrigInjecEventEdge_RisingEdge	 ((uint16_t)0x0040)    /*!<  ADC external trigger rising edge for injected conversion */
#define ADC_ExternalTrigInjecEventEdge_FallingEdge	 ((uint16_t)0x0080)  /*!<  ADC external trigger falling edge for injected conversion */
#define ADC_ExternalTrigInjecEventEdge_BothEdge	     ((uint16_t)0x00C0)  /*!<  ADC external trigger both edges for injected conversion */

/** @defgroup ADC_external_trigger_sources_for_regular_channels_conversion 
  * @{
  */
#define ADC_ExternalTrigConvEvent_0              ((uint16_t)0x0000)   /*!<  ADC external trigger event 0 */
#define ADC_ExternalTrigConvEvent_1              ((uint16_t)0x0040)   /*!<  ADC external trigger event 1 */
#define ADC_ExternalTrigConvEvent_2              ((uint16_t)0x0080)   /*!<  ADC external trigger event 2 */
#define ADC_ExternalTrigConvEvent_3              ((uint16_t)0x00C0)   /*!<  ADC external trigger event 3 */
#define ADC_ExternalTrigConvEvent_4              ((uint16_t)0x0100)   /*!<  ADC external trigger event 4 */
#define ADC_ExternalTrigConvEvent_5              ((uint16_t)0x0140)   /*!<  ADC external trigger event 5 */
#define ADC_ExternalTrigConvEvent_6              ((uint16_t)0x0180)   /*!<  ADC external trigger event 6 */
#define ADC_ExternalTrigConvEvent_7              ((uint16_t)0x01C0)   /*!<  ADC external trigger event 7 */
#define ADC_ExternalTrigConvEvent_8              ((uint16_t)0x0200)   /*!<  ADC external trigger event 8 */
#define ADC_ExternalTrigConvEvent_9              ((uint16_t)0x0240)   /*!<  ADC external trigger event 9 */
#define ADC_ExternalTrigConvEvent_10             ((uint16_t)0x0280)   /*!<  ADC external trigger event 10 */
#define ADC_ExternalTrigConvEvent_11             ((uint16_t)0x02C0)   /*!<  ADC external trigger event 11 */
#define ADC_ExternalTrigConvEvent_12             ((uint16_t)0x0300)   /*!<  ADC external trigger event 12 */
#define ADC_ExternalTrigConvEvent_13             ((uint16_t)0x0340)   /*!<  ADC external trigger event 13 */
#define ADC_ExternalTrigConvEvent_14             ((uint16_t)0x0380)   /*!<  ADC external trigger event 14 */
#define ADC_ExternalTrigConvEvent_15             ((uint16_t)0x03C0)   /*!<  ADC external trigger event 15 */


/** @defgroup ADC_external_trigger_sources_for_Injected_channels_conversion 
  * @{
  */
        
#define ADC_ExternalTrigInjecConvEvent_0              ((uint16_t)0x0000)  /*!<  ADC external trigger for injected conversion event 0 */
#define ADC_ExternalTrigInjecConvEvent_1              ((uint16_t)0x0004)  /*!<  ADC external trigger for injected conversion event 1 */
#define ADC_ExternalTrigInjecConvEvent_2              ((uint16_t)0x0008)  /*!<  ADC external trigger for injected conversion event 2 */
#define ADC_ExternalTrigInjecConvEvent_3              ((uint16_t)0x000C)  /*!<  ADC external trigger for injected conversion event 3 */
#define ADC_ExternalTrigInjecConvEvent_4              ((uint16_t)0x0010)  /*!<  ADC external trigger for injected conversion event 4 */
#define ADC_ExternalTrigInjecConvEvent_5              ((uint16_t)0x0014)  /*!<  ADC external trigger for injected conversion event 5 */
#define ADC_ExternalTrigInjecConvEvent_6              ((uint16_t)0x0018)  /*!<  ADC external trigger for injected conversion event 6 */
#define ADC_ExternalTrigInjecConvEvent_7              ((uint16_t)0x001C)  /*!<  ADC external trigger for injected conversion event 7 */
#define ADC_ExternalTrigInjecConvEvent_8              ((uint16_t)0x0020)  /*!<  ADC external trigger for injected conversion event 8 */
#define ADC_ExternalTrigInjecConvEvent_9              ((uint16_t)0x0024)  /*!<  ADC external trigger for injected conversion event 9 */
#define ADC_ExternalTrigInjecConvEvent_10             ((uint16_t)0x0028)  /*!<  ADC external trigger for injected conversion event 10 */
#define ADC_ExternalTrigInjecConvEvent_11             ((uint16_t)0x002C)  /*!<  ADC external trigger for injected conversion event 11 */
#define ADC_ExternalTrigInjecConvEvent_12             ((uint16_t)0x0030)  /*!<  ADC external trigger for injected conversion event 12 */
#define ADC_ExternalTrigInjecConvEvent_13             ((uint16_t)0x0034)  /*!<  ADC external trigger for injected conversion event 13 */
#define ADC_ExternalTrigInjecConvEvent_14             ((uint16_t)0x0038)  /*!<  ADC external trigger for injected conversion event 14 */
#define ADC_ExternalTrigInjecConvEvent_15             ((uint16_t)0x003C)  /*!<  ADC external trigger for injected conversion event 15 */

/** @defgroup ADC_data_align
  * @{
  */

#define ADC_DataAlign_Right                        ((uint32_t)0x00000000)  /*!<  ADC Data alignment right */
#define ADC_DataAlign_Left                         ((uint32_t)0x00000020)  /*!<  ADC Data alignment left */

/** @defgroup ADC_channels 
  * @{
  */

#define ADC_Channel_1                               ((uint8_t)0x01)    /*!<  ADC Channel 1 */
#define ADC_Channel_2                               ((uint8_t)0x02)    /*!<  ADC Channel 2 */
#define ADC_Channel_3                               ((uint8_t)0x03)    /*!<  ADC Channel 3 */
#define ADC_Channel_4                               ((uint8_t)0x04)    /*!<  ADC Channel 4 */
#define ADC_Channel_5                               ((uint8_t)0x05)    /*!<  ADC Channel 5 */
#define ADC_Channel_6                               ((uint8_t)0x06)    /*!<  ADC Channel 6 */
#define ADC_Channel_7                               ((uint8_t)0x07)    /*!<  ADC Channel 7 */
#define ADC_Channel_8                               ((uint8_t)0x08)    /*!<  ADC Channel 8 */
#define ADC_Channel_9                               ((uint8_t)0x09)    /*!<  ADC Channel 9 */
#define ADC_Channel_10                              ((uint8_t)0x0A)    /*!<  ADC Channel 10 */
#define ADC_Channel_11                              ((uint8_t)0x0B)    /*!<  ADC Channel 11 */
#define ADC_Channel_12                              ((uint8_t)0x0C)    /*!<  ADC Channel 12 */
#define ADC_Channel_13                              ((uint8_t)0x0D)    /*!<  ADC Channel 13 */
#define ADC_Channel_14                              ((uint8_t)0x0E)    /*!<  ADC Channel 14 */
#define ADC_Channel_15                              ((uint8_t)0x0F)    /*!<  ADC Channel 15 */
#define ADC_Channel_16                              ((uint8_t)0x10)    /*!<  ADC Channel 16 */
#define ADC_Channel_17                              ((uint8_t)0x11)    /*!<  ADC Channel 17 */
#define ADC_Channel_18                              ((uint8_t)0x12)    /*!<  ADC Channel 18 */

#define ADC_Channel_TempSensor                      ((uint8_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                         ((uint8_t)ADC_Channel_18)
#define ADC_Channel_Vbat                            ((uint8_t)ADC_Channel_17)


/** @defgroup ADC_mode 
  * @{
  */    
#define ADC_Mode_Independent                  ((uint32_t)0x00000000) /*!<  ADC independent mode */
#define ADC_Mode_CombRegSimulInjSimul         ((uint32_t)0x00000001) /*!<  ADC multi ADC mode: Combined Regular simultaneous injected simultaneous mode */
#define ADC_Mode_CombRegSimulAltTrig          ((uint32_t)0x00000002) /*!<  ADC multi ADC mode: Combined Regular simultaneous Alternate trigger mode */
#define ADC_Mode_InjSimul                     ((uint32_t)0x00000005) /*!<  ADC multi ADC mode: Injected simultaneous mode */
#define ADC_Mode_RegSimul                     ((uint32_t)0x00000006) /*!<  ADC multi ADC mode: Regular simultaneous mode */
#define ADC_Mode_Interleave                   ((uint32_t)0x00000007) /*!<  ADC multi ADC mode: Interleave mode */
#define ADC_Mode_AltTrig                      ((uint32_t)0x00000009) /*!<  ADC multi ADC mode: Alternate Trigger mode */


/** @defgroup ADC_Clock 
  * @{
  */ 
#define ADC_Clock_AsynClkMode                  ((uint32_t)0x00000000)   /*!< ADC Asynchronous clock mode */
#define ADC_Clock_SynClkModeDiv1               ((uint32_t)0x00010000)   /*!< Synchronous clock mode divided by 1 */
#define ADC_Clock_SynClkModeDiv2               ((uint32_t)0x00020000)   /*!<  Synchronous clock mode divided by 2 */
#define ADC_Clock_SynClkModeDiv4               ((uint32_t)0x00030000)   /*!<  Synchronous clock mode divided by 4 */
/** @defgroup ADC_Direct_memory_access_mode_for_multi_mode 
  * @{
  */ 
#define ADC_DMAAccessMode_Disabled      ((uint32_t)0x00000000)     /*!<  DMA mode disabled */
#define ADC_DMAAccessMode_1             ((uint32_t)0x00008000)     /*!<  DMA mode enabled for 12 and 10-bit resolution (6 bit) */
#define ADC_DMAAccessMode_2             ((uint32_t)0x0000C000)     /*!<  DMA mode enabled for 8 and 6-bit resolution (8bit) */

#define ADC_SampleTime_1Cycles5                    ((uint8_t)0x00)   /*!<  ADC sampling time 1.5 cycle */
#define ADC_SampleTime_2Cycles5                    ((uint8_t)0x01)   /*!<  ADC sampling time 2.5 cycles */
#define ADC_SampleTime_4Cycles5                    ((uint8_t)0x02)   /*!<  ADC sampling time 4.5 cycles */
#define ADC_SampleTime_7Cycles5                    ((uint8_t)0x03)   /*!<  ADC sampling time 7.5 cycles */
#define ADC_SampleTime_19Cycles5                   ((uint8_t)0x04)   /*!<  ADC sampling time 19.5 cycles */
#define ADC_SampleTime_61Cycles5                   ((uint8_t)0x05)   /*!<  ADC sampling time 61.5 cycles */
#define ADC_SampleTime_181Cycles5                  ((uint8_t)0x06)   /*!<  ADC sampling time 181.5 cycles */
#define ADC_SampleTime_601Cycles5                  ((uint8_t)0x07)   /*!<  ADC sampling time 601.5 cycles */

/** @defgroup ADC_injected_Channel_selection 
  * @{
  */

#define ADC_InjectedChannel_1                       ADC_Channel_1        /*!<  ADC Injected channel 1 */
#define ADC_InjectedChannel_2                       ADC_Channel_2        /*!<  ADC Injected channel 2 */
#define ADC_InjectedChannel_3                       ADC_Channel_3        /*!<  ADC Injected channel 3 */
#define ADC_InjectedChannel_4                       ADC_Channel_4        /*!<  ADC Injected channel 4 */
#define ADC_InjectedChannel_5                       ADC_Channel_5        /*!<  ADC Injected channel 5 */
#define ADC_InjectedChannel_6                       ADC_Channel_6        /*!<  ADC Injected channel 6 */
#define ADC_InjectedChannel_7                       ADC_Channel_7        /*!<  ADC Injected channel 7 */
#define ADC_InjectedChannel_8                       ADC_Channel_8        /*!<  ADC Injected channel 8 */
#define ADC_InjectedChannel_9                       ADC_Channel_9        /*!<  ADC Injected channel 9 */
#define ADC_InjectedChannel_10                      ADC_Channel_10       /*!<  ADC Injected channel 10 */
#define ADC_InjectedChannel_11                      ADC_Channel_11       /*!<  ADC Injected channel 11 */
#define ADC_InjectedChannel_12                      ADC_Channel_12       /*!<  ADC Injected channel 12 */
#define ADC_InjectedChannel_13                      ADC_Channel_13       /*!<  ADC Injected channel 13 */
#define ADC_InjectedChannel_14                      ADC_Channel_14       /*!<  ADC Injected channel 14 */
#define ADC_InjectedChannel_15                      ADC_Channel_15       /*!<  ADC Injected channel 15 */
#define ADC_InjectedChannel_16                      ADC_Channel_16       /*!<  ADC Injected channel 16 */
#define ADC_InjectedChannel_17                      ADC_Channel_17       /*!<  ADC Injected channel 17 */
#define ADC_InjectedChannel_18                      ADC_Channel_18       /*!<  ADC Injected channel 18 */


#define ADC_InjectedSequence_1                       ADC_Channel_1        /*!<  ADC Injected sequence 1 */
#define ADC_InjectedSequence_2                       ADC_Channel_2        /*!<  ADC Injected sequence 2 */
#define ADC_InjectedSequence_3                       ADC_Channel_3        /*!<  ADC Injected sequence 3 */
#define ADC_InjectedSequence_4                       ADC_Channel_4        /*!<  ADC Injected sequence 4 */
/** @defgroup ADC_analog_watchdog_selection
  * @{
  */

#define ADC_AnalogWatchdog_SingleRegEnable         ((uint32_t)0x00C00000)    /*!<  ADC Analog watchdog single regular mode */
#define ADC_AnalogWatchdog_SingleInjecEnable       ((uint32_t)0x01400000)    /*!<  ADC Analog watchdog single injected mode */
#define ADC_AnalogWatchdog_SingleRegOrInjecEnable  ((uint32_t)0x01C00000)    /*!<  ADC Analog watchdog single regular or injected mode */
#define ADC_AnalogWatchdog_AllRegEnable            ((uint32_t)0x00800000)    /*!<  ADC Analog watchdog all regular mode */
#define ADC_AnalogWatchdog_AllInjecEnable          ((uint32_t)0x01000000)    /*!<  ADC Analog watchdog all injected mode */
#define ADC_AnalogWatchdog_AllRegAllInjecEnable    ((uint32_t)0x01800000)    /*!<  ADC Analog watchdog all regular and all injected mode */
#define ADC_AnalogWatchdog_None                    ((uint32_t)0x00000000)    /*!<  ADC Analog watchdog off */

/** @defgroup ADC_Calibration_Mode_definition 
  * @{
  */
#define ADC_CalibrationMode_Single         ((uint32_t)0x00000000)   /*!<  ADC Calibration for single ended channel */
#define ADC_CalibrationMode_Differential   ((uint32_t)0x40000000)   /*!<  ADC Calibration for differential channel */


/** @defgroup ADC_DMA_Mode_definition 
  * @{
  */
#define ADC_DMAMode_OneShot	   ((uint32_t)0x00000000)   /*!<  ADC DMA Oneshot mode */
#define ADC_DMAMode_Circular   ((uint32_t)0x00000002)   /*!<  ADC DMA circular mode */


/** @defgroup ADC_interrupts_definition 
  * @{
  */

#define ADC_IT_RDY           ((uint16_t)0x0001)  /*!< ADC Ready (ADRDY) interrupt source */
#define ADC_IT_EOSMP         ((uint16_t)0x0002)  /*!< ADC End of Sampling interrupt source */
#define ADC_IT_EOC           ((uint16_t)0x0004)  /*!< ADC End of Regular Conversion interrupt source */
#define ADC_IT_EOS           ((uint16_t)0x0008)  /*!< ADC End of Regular sequence of Conversions interrupt source */
#define ADC_IT_OVR           ((uint16_t)0x0010)  /*!< ADC overrun interrupt source */
#define ADC_IT_JEOC          ((uint16_t)0x0020)  /*!< ADC End of Injected Conversion interrupt source */
#define ADC_IT_JEOS          ((uint16_t)0x0040)  /*!< ADC End of Injected sequence of Conversions interrupt source */
#define ADC_IT_AWD1          ((uint16_t)0x0080)  /*!< ADC Analog watchdog 1 interrupt source */
#define ADC_IT_AWD2          ((uint16_t)0x0100)  /*!< ADC Analog watchdog 2 interrupt source */
#define ADC_IT_AWD3          ((uint16_t)0x0200)  /*!< ADC Analog watchdog 3 interrupt source */
#define ADC_IT_JQOVF         ((uint16_t)0x0400)  /*!< ADC Injected Context Queue Overflow interrupt source */


  
#define ADC_FLAG_RDY           ((uint16_t)0x0001)  /*!< ADC Ready (ADRDY) flag */
#define ADC_FLAG_EOSMP         ((uint16_t)0x0002)  /*!< ADC End of Sampling flag */
#define ADC_FLAG_EOC           ((uint16_t)0x0004)  /*!< ADC End of Regular Conversion flag */
#define ADC_FLAG_EOS           ((uint16_t)0x0008)  /*!< ADC End of Regular sequence of Conversions flag */
#define ADC_FLAG_OVR           ((uint16_t)0x0010)  /*!< ADC overrun flag */
#define ADC_FLAG_JEOC          ((uint16_t)0x0020)  /*!< ADC End of Injected Conversion flag */
#define ADC_FLAG_JEOS          ((uint16_t)0x0040)  /*!< ADC End of Injected sequence of Conversions flag */
#define ADC_FLAG_AWD1          ((uint16_t)0x0080)  /*!< ADC Analog watchdog 1 flag */
#define ADC_FLAG_AWD2          ((uint16_t)0x0100)  /*!< ADC Analog watchdog 2 flag */
#define ADC_FLAG_AWD3          ((uint16_t)0x0200)  /*!< ADC Analog watchdog 3 flag */
#define ADC_FLAG_JQOVF         ((uint16_t)0x0400)  /*!< ADC Injected Context Queue Overflow flag */

/** @defgroup ADC_Common_flags_definition 
  * @{
  */
  
#define ADC_FLAG_MSTRDY           ((uint32_t)0x00000001)  /*!< ADC Master Ready (ADRDY) flag */
#define ADC_FLAG_MSTEOSMP         ((uint32_t)0x00000002)  /*!< ADC Master End of Sampling flag */
#define ADC_FLAG_MSTEOC           ((uint32_t)0x00000004)  /*!< ADC Master End of Regular Conversion flag */
#define ADC_FLAG_MSTEOS           ((uint32_t)0x00000008)  /*!< ADC Master End of Regular sequence of Conversions flag */
#define ADC_FLAG_MSTOVR           ((uint32_t)0x00000010)  /*!< ADC Master overrun flag */
#define ADC_FLAG_MSTJEOC          ((uint32_t)0x00000020)  /*!< ADC Master End of Injected Conversion flag */
#define ADC_FLAG_MSTJEOS          ((uint32_t)0x00000040)  /*!< ADC Master End of Injected sequence of Conversions flag */
#define ADC_FLAG_MSTAWD1          ((uint32_t)0x00000080)  /*!< ADC Master Analog watchdog 1 flag */
#define ADC_FLAG_MSTAWD2          ((uint32_t)0x00000100)  /*!< ADC Master Analog watchdog 2 flag */
#define ADC_FLAG_MSTAWD3          ((uint32_t)0x00000200)  /*!< ADC Master Analog watchdog 3 flag */
#define ADC_FLAG_MSTJQOVF         ((uint32_t)0x00000400)  /*!< ADC Master Injected Context Queue Overflow flag */

#define ADC_FLAG_SLVRDY           ((uint32_t)0x00010000)  /*!< ADC Slave Ready (ADRDY) flag */
#define ADC_FLAG_SLVEOSMP         ((uint32_t)0x00020000)  /*!< ADC Slave End of Sampling flag */
#define ADC_FLAG_SLVEOC           ((uint32_t)0x00040000)  /*!< ADC Slave End of Regular Conversion flag */
#define ADC_FLAG_SLVEOS           ((uint32_t)0x00080000)  /*!< ADC Slave End of Regular sequence of Conversions flag */
#define ADC_FLAG_SLVOVR           ((uint32_t)0x00100000)  /*!< ADC Slave overrun flag */
#define ADC_FLAG_SLVJEOC          ((uint32_t)0x00200000)  /*!< ADC Slave End of Injected Conversion flag */
#define ADC_FLAG_SLVJEOS          ((uint32_t)0x00400000)  /*!< ADC Slave End of Injected sequence of Conversions flag */
#define ADC_FLAG_SLVAWD1          ((uint32_t)0x00800000)  /*!< ADC Slave Analog watchdog 1 flag */
#define ADC_FLAG_SLVAWD2          ((uint32_t)0x01000000)  /*!< ADC Slave Analog watchdog 2 flag */
#define ADC_FLAG_SLVAWD3          ((uint32_t)0x02000000)  /*!< ADC Slave Analog watchdog 3 flag */
#define ADC_FLAG_SLVJQOVF         ((uint32_t)0x04000000)  /*!< ADC Slave Injected Context Queue Overflow flag */
