/*
 * =====================================================================================
 *
 *       Filename:  stm32f30x_rtc.h
 *
 *    Description:  RTC support
 *
 *        Version:  1.0
 *        Created:  09.12.2017 21:46:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#define RTC_HourFormat_24              ((uint32_t)0x00000000)
#define RTC_HourFormat_12              ((uint32_t)0x00000040)
#define RTC_H12_AM                     ((uint8_t)0x00)
#define RTC_H12_PM                     ((uint8_t)0x40)
#define RTC_AlarmDateWeekDaySel_Date      ((uint32_t)0x00000000)
#define RTC_AlarmDateWeekDaySel_WeekDay   ((uint32_t)0x40000000)
#define RTC_AlarmMask_None                ((uint32_t)0x00000000)
#define RTC_AlarmMask_DateWeekDay         ((uint32_t)0x80000000)
#define RTC_AlarmMask_Hours               ((uint32_t)0x00800000)
#define RTC_AlarmMask_Minutes             ((uint32_t)0x00008000)
#define RTC_AlarmMask_Seconds             ((uint32_t)0x00000080)
#define RTC_AlarmMask_All                 ((uint32_t)0x80808080)

#define RTC_Alarm_A                       ((uint32_t)0x00000100)
#define RTC_Alarm_B                       ((uint32_t)0x00000200)

#define RTC_AlarmSubSecondMask_All         ((uint32_t)0x00000000) /*!< All Alarm SS fields are masked. 
                                                                       There is no comparison on sub seconds 
                                                                       for Alarm */
#define RTC_AlarmSubSecondMask_SS14_1      ((uint32_t)0x01000000) /*!< SS[14:1] are don't care in Alarm 
                                                                       comparison. Only SS[0] is compared. */
#define RTC_AlarmSubSecondMask_SS14_2      ((uint32_t)0x02000000) /*!< SS[14:2] are don't care in Alarm 
                                                                       comparison. Only SS[1:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_3      ((uint32_t)0x03000000) /*!< SS[14:3] are don't care in Alarm 
                                                                       comparison. Only SS[2:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_4      ((uint32_t)0x04000000) /*!< SS[14:4] are don't care in Alarm 
                                                                       comparison. Only SS[3:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_5      ((uint32_t)0x05000000) /*!< SS[14:5] are don't care in Alarm 
                                                                       comparison. Only SS[4:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_6      ((uint32_t)0x06000000) /*!< SS[14:6] are don't care in Alarm 
                                                                       comparison. Only SS[5:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_7      ((uint32_t)0x07000000) /*!< SS[14:7] are don't care in Alarm 
                                                                       comparison. Only SS[6:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_8      ((uint32_t)0x08000000) /*!< SS[14:8] are don't care in Alarm 
                                                                       comparison. Only SS[7:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_9      ((uint32_t)0x09000000) /*!< SS[14:9] are don't care in Alarm 
                                                                       comparison. Only SS[8:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_10     ((uint32_t)0x0A000000) /*!< SS[14:10] are don't care in Alarm 
                                                                       comparison. Only SS[9:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_11     ((uint32_t)0x0B000000) /*!< SS[14:11] are don't care in Alarm 
                                                                       comparison. Only SS[10:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_12     ((uint32_t)0x0C000000) /*!< SS[14:12] are don't care in Alarm 
                                                                       comparison.Only SS[11:0] are compared */
#define RTC_AlarmSubSecondMask_SS14_13     ((uint32_t)0x0D000000) /*!< SS[14:13] are don't care in Alarm 
                                                                       comparison. Only SS[12:0] are compared */
#define RTC_AlarmSubSecondMask_SS14        ((uint32_t)0x0E000000) /*!< SS[14] is don't care in Alarm 
                                                                       comparison.Only SS[13:0] are compared */
#define RTC_AlarmSubSecondMask_None        ((uint32_t)0x0F000000) /*!< SS[14:0] are compared and must match 
                                                                       to activate alarm. */
#define RTC_WakeUpClock_RTCCLK_Div16        ((uint32_t)0x00000000)
#define RTC_WakeUpClock_RTCCLK_Div8         ((uint32_t)0x00000001)
#define RTC_WakeUpClock_RTCCLK_Div4         ((uint32_t)0x00000002)
#define RTC_WakeUpClock_RTCCLK_Div2         ((uint32_t)0x00000003)
#define RTC_WakeUpClock_CK_SPRE_16bits      ((uint32_t)0x00000004)
#define RTC_WakeUpClock_CK_SPRE_17bits      ((uint32_t)0x00000006)


#define RTC_TimeStampEdge_Rising          ((uint32_t)0x00000000)
#define RTC_TimeStampEdge_Falling         ((uint32_t)0x00000008)

/** @defgroup RTC_Output_selection_Definitions 
  * @{
  */ 
#define RTC_Output_Disable             ((uint32_t)0x00000000)
#define RTC_Output_AlarmA              ((uint32_t)0x00200000)
#define RTC_Output_AlarmB              ((uint32_t)0x00400000)
#define RTC_Output_WakeUp              ((uint32_t)0x00600000)

#define RTC_OutputPolarity_High           ((uint32_t)0x00000000)
#define RTC_OutputPolarity_Low            ((uint32_t)0x00100000)


#define RTC_CalibOutput_512Hz            ((uint32_t)0x00000000) 
#define RTC_CalibOutput_1Hz              ((uint32_t)0x00080000)

#define RTC_SmoothCalibPeriod_32sec   ((uint32_t)0x00000000) /*!<  if RTCCLK = 32768 Hz, Smooth calibation
                                                             period is 32s,  else 2exp20 RTCCLK seconds */
#define RTC_SmoothCalibPeriod_16sec   ((uint32_t)0x00002000) /*!<  if RTCCLK = 32768 Hz, Smooth calibation 
                                                             period is 16s, else 2exp19 RTCCLK seconds */
#define RTC_SmoothCalibPeriod_8sec    ((uint32_t)0x00004000) /*!<  if RTCCLK = 32768 Hz, Smooth calibation 
                                                             period is 8s, else 2exp18 RTCCLK seconds */
/** @defgroup RTC_Smooth_calib_Plus_pulses_Definitions 
  * @{
  */ 
#define RTC_SmoothCalibPlusPulses_Set    ((uint32_t)0x00008000) /*!<  The number of RTCCLK pulses added  
                                                                during a X -second window = Y - CALM[8:0]. 
                                                                 with Y = 512, 256, 128 when X = 32, 16, 8 */
#define RTC_SmoothCalibPlusPulses_Reset  ((uint32_t)0x00000000) /*!<  The number of RTCCLK pulses subbstited
                                                                 during a 32-second window =   CALM[8:0]. */

#define RTC_DayLightSaving_SUB1H   ((uint32_t)0x00020000)
#define RTC_DayLightSaving_ADD1H   ((uint32_t)0x00010000)

#define RTC_StoreOperation_Reset        ((uint32_t)0x00000000)
#define RTC_StoreOperation_Set          ((uint32_t)0x00040000)

#define RTC_TamperTrigger_RisingEdge            ((uint32_t)0x00000000)
#define RTC_TamperTrigger_FallingEdge           ((uint32_t)0x00000001)
#define RTC_TamperTrigger_LowLevel              ((uint32_t)0x00000000)
#define RTC_TamperTrigger_HighLevel             ((uint32_t)0x00000001)

#define RTC_TamperFilter_Disable   ((uint32_t)0x00000000) /*!< Tamper filter is disabled */

#define RTC_TamperFilter_2Sample   ((uint32_t)0x00000800) /*!< Tamper is activated after 2 
                                                          consecutive samples at the active level */
#define RTC_TamperFilter_4Sample   ((uint32_t)0x00001000) /*!< Tamper is activated after 4 
                                                          consecutive samples at the active level */
#define RTC_TamperFilter_8Sample   ((uint32_t)0x00001800) /*!< Tamper is activated after 8 
                                                          consecutive samples at the active leve. */
/** @defgroup RTC_Tamper_Sampling_Frequencies_Definitions 
  * @{
  */ 
#define RTC_TamperSamplingFreq_RTCCLK_Div32768  ((uint32_t)0x00000000) /*!< Each of the tamper inputs are sampled
                                                                           with a frequency =  RTCCLK / 32768 */
#define RTC_TamperSamplingFreq_RTCCLK_Div16384  ((uint32_t)0x000000100) /*!< Each of the tamper inputs are sampled
                                                                            with a frequency =  RTCCLK / 16384 */
#define RTC_TamperSamplingFreq_RTCCLK_Div8192   ((uint32_t)0x00000200) /*!< Each of the tamper inputs are sampled
                                                                           with a frequency =  RTCCLK / 8192  */
#define RTC_TamperSamplingFreq_RTCCLK_Div4096   ((uint32_t)0x00000300) /*!< Each of the tamper inputs are sampled
                                                                           with a frequency =  RTCCLK / 4096  */
#define RTC_TamperSamplingFreq_RTCCLK_Div2048   ((uint32_t)0x00000400) /*!< Each of the tamper inputs are sampled
                                                                           with a frequency =  RTCCLK / 2048  */
#define RTC_TamperSamplingFreq_RTCCLK_Div1024   ((uint32_t)0x00000500) /*!< Each of the tamper inputs are sampled
                                                                           with a frequency =  RTCCLK / 1024  */
#define RTC_TamperSamplingFreq_RTCCLK_Div512    ((uint32_t)0x00000600) /*!< Each of the tamper inputs are sampled
                                                                           with a frequency =  RTCCLK / 512   */
#define RTC_TamperSamplingFreq_RTCCLK_Div256    ((uint32_t)0x00000700) /*!< Each of the tamper inputs are sampled
                                                                           with a frequency =  RTCCLK / 256   */

/** @defgroup RTC_Tamper_Pins_Definitions 
  * @{
  */ 
#define RTC_Tamper_1            RTC_TAFCR_TAMP1E /*!< Tamper detection enable for 
                                                      input tamper 1 */
#define RTC_Tamper_2            RTC_TAFCR_TAMP2E /*!< Tamper detection enable for 
                                                      input tamper 2 */
#define RTC_Tamper_3            RTC_TAFCR_TAMP3E /*!< Tamper detection enable for 
                                                      input tamper 3 */
/** @defgroup RTC_Output_Type_ALARM_OUT 
  * @{
  */ 
#define RTC_OutputType_OpenDrain           ((uint32_t)0x00000000)
#define RTC_OutputType_PushPull            ((uint32_t)0x00040000)


/** @defgroup RTC_Add_1_Second_Parameter_Definitions
  * @{
  */ 
#define RTC_ShiftAdd1S_Reset      ((uint32_t)0x00000000)
#define RTC_ShiftAdd1S_Set        ((uint32_t)0x80000000)

/** @defgroup RTC_Backup_Registers_Definitions 
  * @{
  */

#define RTC_BKP_DR0                       ((uint32_t)0x00000000)
#define RTC_BKP_DR1                       ((uint32_t)0x00000001)
#define RTC_BKP_DR2                       ((uint32_t)0x00000002)
#define RTC_BKP_DR3                       ((uint32_t)0x00000003)
#define RTC_BKP_DR4                       ((uint32_t)0x00000004)
#define RTC_BKP_DR5                       ((uint32_t)0x00000005)
#define RTC_BKP_DR6                       ((uint32_t)0x00000006)
#define RTC_BKP_DR7                       ((uint32_t)0x00000007)
#define RTC_BKP_DR8                       ((uint32_t)0x00000008)
#define RTC_BKP_DR9                       ((uint32_t)0x00000009)
#define RTC_BKP_DR10                      ((uint32_t)0x0000000A)
#define RTC_BKP_DR11                      ((uint32_t)0x0000000B)
#define RTC_BKP_DR12                      ((uint32_t)0x0000000C)
#define RTC_BKP_DR13                      ((uint32_t)0x0000000D)
#define RTC_BKP_DR14                      ((uint32_t)0x0000000E)
#define RTC_BKP_DR15                      ((uint32_t)0x0000000F)

#define RTC_Format_BIN                    ((uint32_t)0x000000000)
#define RTC_Format_BCD                    ((uint32_t)0x000000001)

/** @defgroup RTC_Flags_Definitions 
  * @{
  */ 
#define RTC_FLAG_RECALPF                  ((uint32_t)0x00010000)
#define RTC_FLAG_TAMP3F                   ((uint32_t)0x00008000)
#define RTC_FLAG_TAMP2F                   ((uint32_t)0x00004000)
#define RTC_FLAG_TAMP1F                   ((uint32_t)0x00002000)
#define RTC_FLAG_TSOVF                    ((uint32_t)0x00001000)
#define RTC_FLAG_TSF                      ((uint32_t)0x00000800)
#define RTC_FLAG_WUTF                     ((uint32_t)0x00000400)
#define RTC_FLAG_ALRBF                    ((uint32_t)0x00000200)
#define RTC_FLAG_ALRAF                    ((uint32_t)0x00000100)
#define RTC_FLAG_INITF                    ((uint32_t)0x00000040)
#define RTC_FLAG_RSF                      ((uint32_t)0x00000020)
#define RTC_FLAG_INITS                    ((uint32_t)0x00000010)
#define RTC_FLAG_SHPF                     ((uint32_t)0x00000008)
#define RTC_FLAG_WUTWF                    ((uint32_t)0x00000004)
#define RTC_FLAG_ALRBWF                   ((uint32_t)0x00000002)
#define RTC_FLAG_ALRAWF                   ((uint32_t)0x00000001)


/** @defgroup RTC_Interrupts_Definitions 
  * @{
  */ 
#define RTC_IT_TS                         ((uint32_t)0x00008000)
#define RTC_IT_WUT                        ((uint32_t)0x00004000)
#define RTC_IT_ALRB                       ((uint32_t)0x00002000)
#define RTC_IT_ALRA                       ((uint32_t)0x00001000)
#define RTC_IT_TAMP                       ((uint32_t)0x00000004) /* Used only to Enable the Tamper Interrupt */
#define RTC_IT_TAMP1                      ((uint32_t)0x00020000)
#define RTC_IT_TAMP2                      ((uint32_t)0x00040000)
#define RTC_IT_TAMP3                      ((uint32_t)0x00080000)
