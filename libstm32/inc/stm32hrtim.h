#pragma once

#include <stm32lib.h>

#ifdef __cplusplus
extern "C" {
namespace stm32 {
#endif


/**
  * @brief  HRTIM Configuration Structure definition - Time base related parameters
  */
typedef struct HRTIM_BaseInit
{
  uint32_t Period;                 /*!< Specifies the timer period
                                        The period value must be above 3 periods of the fHRTIM clock.
                                        Maximum value is = 0xFFDF */
  uint32_t RepetitionCounter;      /*!< Specifies the timer repetition period
                                        This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF. */
  uint32_t PrescalerRatio;         /*!< Specifies the timer clock prescaler ratio.
                                        This parameter can be any value of @ref HRTIM_PrescalerRatio   */
  uint32_t Mode;                   /*!< Specifies the counter operating mode
                                        This parameter can be any value of @ref HRTIM_Mode   */
} HRTIM_BaseInitTypeDef;


/** 
  * @brief  Waveform mode initialization parameters definition
  */
typedef struct HRTIM_TimerInit {
  uint32_t HalfModeEnable;    /*!< Specifies whether or not half mode is enabled
                                   This parameter can be a combination of @ref HRTIM_HalfModeEnable  */
  uint32_t StartOnSync;       /*!< Specifies whether or not timer is reset by a rising edge on the synchronization input (when enabled)
                                   This parameter can be a combination of @ref HRTIM_StartOnSyncInputEvent  */
  uint32_t ResetOnSync;        /*!< Specifies whether or not timer is reset by a rising edge on the synchronization input (when enabled)
                                   This parameter can be a combination of @ref HRTIM_ResetOnSyncInputEvent  */
  uint32_t DACSynchro;        /*!< Indicates whether or not the a DAC synchronization event is generated 
                                   This parameter can be any value of @ref HRTIM_DACSynchronization   */
  uint32_t PreloadEnable;     /*!< Specifies whether or not register preload is enabled
                                   This parameter can be a combination of @ref HRTIM_RegisterPreloadEnable  */
  uint32_t UpdateGating;      /*!< Specifies how the update occurs with respect to a burst DMA transaction or
                                   update enable inputs (Slave timers only)  
                                   This parameter can be any value of @ref HRTIM_UpdateGating   */
  uint32_t BurstMode;         /*!< Specifies how the timer behaves during a burst mode operation
                                    This parameter can be a combination of @ref HRTIM_TimerBurstMode  */
  uint32_t RepetitionUpdate;  /*!< Specifies whether or not registers update is triggered by the repetition event 
                                   This parameter can be a combination of @ref HRTIM_TimerRepetitionUpdate */
} HRTIM_TimerInitTypeDef;


/** 
  * @brief  Basic output compare mode configuration definition
  */
typedef struct HRTIM_BasicOCChannelCfg {
  uint32_t Mode;       /*!< Specifies the output compare mode (toggle, active, inactive)
                            This parameter can be a combination of @ref HRTIM_BasicOCMode */ 
  uint32_t Pulse;      /*!< Specifies the compare value to be loaded into the Compare Register. 
                            The compare value must be above or equal to 3 periods of the fHRTIM clock */
  uint32_t Polarity;   /*!< Specifies the output polarity 
                            This parameter can be any value of @ref HRTIM_Output_Polarity */
  uint32_t IdleState;  /*!< Specifies whether the output level is active or inactive when in IDLE state  
                            This parameter can be any value of @ref HRTIM_OutputIDLEState */
} HRTIM_BasicOCChannelCfgTypeDef;

/** 
  * @brief  Basic PWM output mode configuration definition
  */
typedef struct HRTIM_BasicPWMChannelCfg {
  uint32_t Pulse;            /*!< Specifies the compare value to be loaded into the Compare Register. 
                                  The compare value must be above or equal to 3 periods of the fHRTIM clock */
  uint32_t Polarity;        /*!< Specifies the output polarity 
                                 This parameter can be any value of @ref HRTIM_OutputPolarity */
  uint32_t IdleState;       /*!< Specifies whether the output level is active or inactive when in IDLE state  
                                 This parameter can be any value of @ref HRTIM_OutputIDLEState */
} HRTIM_BasicPWMChannelCfgTypeDef;

/**
  * @brief  Basic capture mode configuration definition
  */
typedef struct HRTIM_BasicCaptureChannelCfg {
  uint32_t CaptureUnit;      /*!< Specifies the external event Channel 
                                   This parameter can be any 'EEVx' value of @ref HRTIM_CaptureUnit */
  uint32_t Event;             /*!< Specifies the external event triggering the capture 
                                   This parameter can be any 'EEVx' value of @ref HRTIM_ExternalEventChannels */
  uint32_t EventPolarity;     /*!< Specifies the polarity of the external event (in case of level sensitivity) 
                                   This parameter can be a value of @ref HRTIM_ExternalEventPolarity */ 
  uint32_t EventSensitivity;  /*!< Specifies the sensitivity of the external event 
                                   This parameter can be a value of @ref HRTIM_ExternalEventSensitivity */ 
  uint32_t EventFilter;       /*!< Defines the frequency used to sample the External Event and the length of the digital filter 
                                   This parameter can be a value of @ref HRTIM_ExternalEventFilter */ 
} HRTIM_BasicCaptureChannelCfgTypeDef;

/**
  * @brief  Basic One Pulse mode configuration definition
  */
typedef struct HRTIM_BasicOnePulseChannelCfg {
  uint32_t Pulse;             /*!< Specifies the compare value to be loaded into the Compare Register. 
                                   The compare value must be above or equal to 3 periods of the fHRTIM clock */
  uint32_t OutputPolarity;    /*!< Specifies the output polarity 
                                   This parameter can be any value of @ref HRTIM_Output_Polarity */
  uint32_t OutputIdleState;   /*!< Specifies whether the output level is active or inactive when in IDLE state  
                                   This parameter can be any value of @ref HRTIM_Output_IDLE_State */
  uint32_t Event;             /*!< Specifies the external event triggering the pulse generation 
                                   This parameter can be any 'EEVx' value of @ref HRTIM_Capture_Unit_Trigger */
  uint32_t EventPolarity;     /*!< Specifies the polarity of the external event (in case of level sensitivity) 
                                   This parameter can be a value of @ref HRTIM_ExternalEventPolarity */ 
  uint32_t EventSensitivity;  /*!< Specifies the sensitivity of the external event 
                                   This parameter can be a value of @ref HRTIM_ExternalEventSensitivity */ 
  uint32_t EventFilter;       /*!< Defines the frequency used to sample the External Event and the length of the digital filter 
                                   This parameter can be a value of @ref HRTIM_ExternalEventFilter */ 
} HRTIM_BasicOnePulseChannelCfgTypeDef;

/**
  * @brief  Timer configuration definition
  */
typedef struct HRTIM_TimerCfg {
  uint32_t PushPull;                  /*!< Specifies whether or not the push-pull mode is enabled
                                           This parameter can be a value of @ref HRTIM_TimerPushPullMode */
  uint32_t FaultEnable;               /*!< Specifies which fault channels are enabled for the timer
                                           This parameter can be a combination of @ref HRTIM_TimerFaultEnabling  */
  uint32_t FaultLock;                 /*!< Specifies whether or not fault enabling status is write protected
                                           This parameter can be a value of @ref HRTIM_TimerFaultLock */
  uint32_t DeadTimeInsertion;         /*!< Specifies whether or not dead time insertion is enabled for the timer
                                           This parameter can be a value of @ref HRTIM_TimerDeadtimeInsertion */
  uint32_t DelayedProtectionMode;     /*!< Specifies the delayed protection mode 
                                          This parameter can be a value of @ref HRTIM_TimerDelayedProtectionMode */
  uint32_t UpdateTrigger;             /*!< Specifies source(s) triggering the timer registers update 
                                            This parameter can be a combination of @ref HRTIM_TimerUpdateTrigger */
  uint32_t ResetTrigger;              /*!< Specifies source(s) triggering the timer counter reset 
                                           This parameter can be a combination of @ref HRTIM_TimerResetTrigger */
  uint32_t ResetUpdate;              /*!< Specifies whether or not registers update is triggered when the timer counter is reset 
                                           This parameter can be a combination of @ref HRTIM_TimerResetUpdate */
} HRTIM_TimerCfgTypeDef;

/** 
  * @brief  Compare unit configuration definition
  */
typedef struct {
  uint32_t CompareValue;         /*!< Specifies the compare value of the timer compare unit 
                                      the minimum value must be greater than or equal to 3 periods of the fHRTIM clock
                                      the maximum value must be less than or equal to 0xFFFF - 1 periods of the fHRTIM clock */
  uint32_t AutoDelayedMode;      /*!< Specifies the auto delayed mode for compare unit 2 or 4 
                                      This parameter can be a value of @ref HRTIM_CompareUnitAutoDelayedMode */
  uint32_t AutoDelayedTimeout;   /*!< Specifies compare value for timing unit 1 or 3 when auto delayed mode with time out is selected 
                                      CompareValue +  AutoDelayedTimeout must be less than 0xFFFF */
} HRTIM_CompareCfgTypeDef;

/** 
  * @brief  Capture unit configuration definition
  */
typedef struct HRTIM_CaptureCfg {
  uint32_t Trigger;   /*!< Specifies source(s) triggering the capture 
                           This parameter can be a combination of @ref HRTIM_CaptureUnitTrigger */
} HRTIM_CaptureCfgTypeDef;

/** 
  * @brief  Output configuration definition
  */
typedef struct HRTIM_OutputCfg {
  uint32_t Polarity;              /*!< Specifies the output polarity 
                                       This parameter can be any value of @ref HRTIM_Output_Polarity */
  uint32_t SetSource;             /*!< Specifies the event(s) transitioning the output from its inactive level to its active level  
                                       This parameter can be any value of @ref HRTIM_OutputSetSource */
  uint32_t ResetSource;           /*!< Specifies the event(s) transitioning the output from its active level to its inactive level  
                                       This parameter can be any value of @ref HRTIM_OutputResetSource */
  uint32_t IdleMode;              /*!< Specifies whether or not the output is affected by a burst mode operation  
                                       This parameter can be any value of @ref HRTIM_OutputIdleMode */
  uint32_t IdleState;             /*!< Specifies whether the output level is active or inactive when in IDLE state  
                                       This parameter can be any value of @ref HRTIM_OutputIDLEState */
  uint32_t FaultState;            /*!< Specifies whether the output level is active or inactive when in FAULT state  
                                       This parameter can be any value of @ref HRTIM_OutputFAULTState */
  uint32_t ChopperModeEnable;     /*!< Indicates whether or not the chopper mode is enabled 
                                       This parameter can be any value of @ref HRTIM_OutputChopperModeEnable */
  uint32_t BurstModeEntryDelayed;  /* !<Indicates whether or not deadtime is inserted when entering the IDLE state
                                        during a burst mode operation
                                        This parameters can be any value of @ref HRTIM_OutputBurstModeEntryDelayed */
} HRTIM_OutputCfgTypeDef;

/**
  * @brief  External event filtering in timing units configuration definition
  */
typedef struct HRTIM_TimerEventFilteringCfg {
  uint32_t Filter;       /*!< Specifies the type of event filtering within the timing unit 
                             This parameter can be a value of @ref HRTIM_TimerExternalEventFilter */ 
  uint32_t Latch;       /*!< Specifies whether or not the signal is latched
                             This parameter can be a value of @ref HRTIM_TimerExternalEventLatch */
} HRTIM_TimerEventFilteringCfgTypeDef;

/** 
  * @brief  Dead time feature configuration definition
  */
typedef struct {
  uint32_t Prescaler;       /*!< Specifies the Deadtime Prescaler 
                                 This parameter can be a number between 0x0 and = 0x7 */ 
  uint32_t RisingValue;     /*!< Specifies the Deadtime following a rising edge 
                                 This parameter can be a number between 0x0 and 0xFF */ 
  uint32_t RisingSign;      /*!< Specifies whether the deadtime is positive or negative on rising edge
                                 This parameter can be a value of @ref HRTIM_DeadtimeRisingSign */ 
  uint32_t RisingLock;      /*!< Specifies whether or not deadtime rising settings (value and sign) are write protected 
                                 This parameter can be a value of @ref HRTIM_DeadtimeRisingLock */ 
  uint32_t RisingSignLock;  /*!< Specifies whether or not deadtime rising sign is write protected 
                                 This parameter can be a value of @ref HRTIM_DeadtimeRisingSignLock */ 
  uint32_t FallingValue;    /*!< Specifies the Deadtime following a falling edge 
                                This parameter can be a number between 0x0 and 0xFF */ 
  uint32_t FallingSign;     /*!< Specifies whether the deadtime is positive or negative on falling edge 
                                This parameter can be a value of @ref HRTIM_DeadtimeFallingSign */ 
  uint32_t FallingLock;     /*!< Specifies whether or not deadtime falling settings (value and sign) are write protected 
                                This parameter can be a value of @ref HRTIM_DeadtimeFallingLock */ 
  uint32_t FallingSignLock; /*!< Specifies whether or not deadtime falling sign is write protected 
                                This parameter can be a value of @ref HRTIM_DeadtimeFallingSignLock */ 
} HRTIM_DeadTimeCfgTypeDef;

/** 
  * @brief  Chopper mode configuration definition
  */
typedef struct HRTIM_ChopperModeCfg {
  uint32_t CarrierFreq;  /*!< Specifies the Timer carrier frequency value.
                              This parameter can be a value between 0 and 0xF */
  uint32_t DutyCycle;   /*!< Specifies the Timer chopper duty cycle value.
                             This parameter can be a value between 0 and 0x7 */
  uint32_t StartPulse;  /*!< Specifies the Timer pulse width value.
                             This parameter can be a value between 0 and 0xF */
} HRTIM_ChopperModeCfgTypeDef;

/**
  * @brief  Master synchronization configuration definition
  */
typedef struct HRTIM_SynchroCfg {
  uint32_t SyncInputSource;     /*!< Specifies the external synchronization input source 
                                     This parameter can be a value of @ref HRTIM_SynchronizationInputSource */
  uint32_t SyncOutputSource;    /*!< Specifies the source and event to be sent on the external synchronization outputs 
                                     This parameter can be a value of @ref HRTIM_SynchronizationOutputSource */
  uint32_t SyncOutputPolarity;  /*!< Specifies the conditioning of the event to be sent on the external synchronization outputs 
                                     This parameter can be a value of @ref HRTIM_SynchronizationOutputPolarity */
} HRTIM_SynchroCfgTypeDef;

/** 
  * @brief  External event channel configuration definition
  */ 
typedef struct HRTIM_EventCfg {
  uint32_t Source;        /*!< Identifies the source of the external event 
                                This parameter can be a value of @ref HRTIM_ExternalEventSources */ 
  uint32_t Polarity;      /*!< Specifies the polarity of the external event (in case of level sensitivity) 
                               This parameter can be a value of @ref HRTIM_ExternalEventPolarity */ 
  uint32_t Sensitivity;   /*!< Specifies the sensitivity of the external event 
                               This parameter can be a value of @ref HRTIM_ExternalEventSensitivity */ 
  uint32_t Filter;        /*!< Defines the frequency used to sample the External Event and the length of the digital filter 
                               This parameter can be a value of @ref HRTIM_ExternalEventFilter */ 
  uint32_t FastMode;     /*!< Indicates whether or not low latency mode is enabled for the external event 
                              This parameter can be a value of @ref HRTIM_ExternalEventFastMode */
} HRTIM_EventCfgTypeDef;

/** 
  * @brief  Fault channel configuration definition
  */ 
typedef struct HRTIM_FaultCfg {
  uint32_t Source;        /*!< Identifies the source of the fault 
                                This parameter can be a value of @ref HRTIM_FaultSources */ 
  uint32_t Polarity;      /*!< Specifies the polarity of the fault event 
                               This parameter can be a value of @ref HRTIM_FaultPolarity */ 
  uint32_t Filter;        /*!< Defines the frequency used to sample the Fault input and the length of the digital filter 
                               This parameter can be a value of @ref HRTIM_FaultFilter */ 
  uint32_t Lock;          /*!< Indicates whether or not fault programming bits are write protected 
                              This parameter can be a value of @ref HRTIM_FaultLock */
} HRTIM_FaultCfgTypeDef;

/** 
  * @brief  Burst mode configuration definition
  */
typedef struct HRTIM_BurstModeCfg {
  uint32_t Mode;           /*!< Specifies the burst mode operating mode
                                This parameter can be a value of @ref HRTIM_BurstModeOperatingMode */
  uint32_t ClockSource;    /*!< Specifies the burst mode clock source
                                This parameter can be a value of @ref HRTIM_BurstModeClockSource */
  uint32_t Prescaler;      /*!< Specifies the burst mode prescaler
                                This parameter can be a value of @ref HRTIM_BurstModePrescaler */
  uint32_t PreloadEnable;  /*!< Specifies whether or not preload is enabled for burst mode related registers (HRTIM_BMCMPR and HRTIM_BMPER)
                                This parameter can be a combination of @ref HRTIM_BurstModeRegisterPreloadEnable  */
  uint32_t Trigger;        /*!< Specifies the event(s) triggering the burst operation 
                                This parameter can be a combination of @ref HRTIM_BurstModeTrigger  */
  uint32_t IdleDuration;   /*!< Specifies number of periods during which the selected timers are in idle state 
                                This parameter can be a number between 0x0 and 0xFFFF  */
  uint32_t Period;        /*!< Specifies burst mode repetition period 
                                This parameter can be a number between 0x1 and 0xFFFF  */
} HRTIM_BurstModeCfgTypeDef;

/** 
  * @brief  ADC trigger configuration definition
  */
typedef struct HRTIM_ADCTriggerCfg {
  uint32_t UpdateSource;  /*!< Specifies the ADC trigger update source  
                               This parameter can be a combination of @ref HRTIM_ADCTriggerUpdateSource  */
  uint32_t Trigger;      /*!< Specifies the event(s) triggering the ADC conversion  
                              This parameter can be a combination of @ref HRTIM_ADCTriggerEvent  */
} HRTIM_ADCTriggerCfgTypeDef;



/* Simple time base related functions  *****************************************/
void HRTIM_SimpleBase_Init(HRTIM_TypeDef* HRTIMx, uint32_t TimerIdx, HRTIM_BaseInitTypeDef* HRTIM_BaseInitStruct);

void HRTIM_DeInit(HRTIM_TypeDef* HRTIMx);

void HRTIM_SimpleBaseStart(HRTIM_TypeDef *HRTIMx, uint32_t TimerIdx);
void HRTIM_SimpleBaseStop(HRTIM_TypeDef *HRTIMx, uint32_t TimerIdx);

/* Simple output compare related functions  ************************************/
void HRTIM_SimpleOC_Init(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, HRTIM_BaseInitTypeDef* HRTIM_BaseInitStruct);

void HRTIM_SimpleOCChannelConfig(HRTIM_TypeDef *HRTIMx,
                                                 uint32_t TimerIdx,
                                                 uint32_t OCChannel,
                                                 HRTIM_BasicOCChannelCfgTypeDef* pBasicOCChannelCfg);

void HRTIM_SimpleOCStart(HRTIM_TypeDef *HRTIMx,
                                         uint32_t TimerIdx,
                                         uint32_t OCChannel);
void HRTIM_SimpleOCStop(HRTIM_TypeDef * HRTIMx,
                                        uint32_t TimerIdx,
                                        uint32_t OCChannel);
/* Simple PWM output related functions  ****************************************/
void HRTIM_SimplePWM_Init(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, HRTIM_BaseInitTypeDef* HRTIM_BaseInitStruct);

void HRTIM_SimplePWMChannelConfig(HRTIM_TypeDef *HRTIMx,
                                                  uint32_t TimerIdx,
                                                  uint32_t PWMChannel,
                                                  HRTIM_BasicPWMChannelCfgTypeDef* pBasicPWMChannelCfg);

void HRTIM_SimplePWMStart(HRTIM_TypeDef * HRTIMx,
                                          uint32_t TimerIdx,
                                          uint32_t PWMChannel);
void HRTIM_SimplePWMStop(HRTIM_TypeDef * HRTIMx,
                                         uint32_t TimerIdx,
                                         uint32_t PWMChannel);
/* Simple capture related functions  *******************************************/
void HRTIM_SimpleCapture_Init(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, HRTIM_BaseInitTypeDef* HRTIM_BaseInitStruct);

void HRTIM_SimpleCaptureChannelConfig(HRTIM_TypeDef *HRTIMx,
                                                      uint32_t TimerIdx,
                                                      uint32_t CaptureChannel,
                                                      HRTIM_BasicCaptureChannelCfgTypeDef* pBasicCaptureChannelCfg);

void HRTIM_SimpleCaptureStart(HRTIM_TypeDef * HRTIMx,
                                              uint32_t TimerIdx,
                                              uint32_t CaptureChannel);
void HRTIM_SimpleCaptureStop(HRTIM_TypeDef * HRTIMx,
                                             uint32_t TimerIdx,
                                             uint32_t CaptureChannel);
/* SImple one pulse related functions  *****************************************/
void HRTIM_SimpleOnePulse_Init(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, HRTIM_BaseInitTypeDef* HRTIM_BaseInitStruct);

void HRTIM_SimpleOnePulseChannelConfig(HRTIM_TypeDef *HRTIMx,
                                                       uint32_t TimerIdx,
                                                       uint32_t OnePulseChannel,
                                                       HRTIM_BasicOnePulseChannelCfgTypeDef* pBasicOnePulseChannelCfg);

void HRTIM_SimpleOnePulseStart(HRTIM_TypeDef * HRTIMx,
                                                uint32_t TimerIdx,
                                                uint32_t OnePulseChannel);
void HRTIM_SimpleOnePulseStop(HRTIM_TypeDef * HRTIM_,
                                              uint32_t TimerIdx,
                                              uint32_t OnePulseChannel);
/* Waveform related functions *************************************************/
void HRTIM_Waveform_Init(HRTIM_TypeDef * HRTIMx,
                                         uint32_t TimerIdx,
                                         HRTIM_BaseInitTypeDef* HRTIM_BaseInitStruct,
                                         HRTIM_TimerInitTypeDef* HRTIM_TimerInitStruct);

void HRTIM_WaveformTimerConfig(HRTIM_TypeDef *HRTIMx,
                                                uint32_t TimerIdx,
                                                HRTIM_TimerCfgTypeDef * HRTIM_TimerCfgStruct);

void HRTIM_WaveformCompareConfig(HRTIM_TypeDef *HRTIMx,
                                                  uint32_t TimerIdx,
                                                  uint32_t CompareUnit,
                                                  HRTIM_CompareCfgTypeDef* pCompareCfg);

void HRTIM_SlaveSetCompare(HRTIM_TypeDef * HRTIMx,
                                                  uint32_t TimerIdx,
                                                  uint32_t CompareUnit,
                                                  uint32_t Compare);

void HRTIM_MasterSetCompare(HRTIM_TypeDef * HRTIMx,
                                                  uint32_t CompareUnit,
                                                  uint32_t Compare);

void HRTIM_WaveformCaptureConfig(HRTIM_TypeDef *HRTIMx,
                                                  uint32_t TimerIdx,
                                                  uint32_t CaptureUnit,
                                                  HRTIM_CaptureCfgTypeDef* pCaptureCfg);

void HRTIM_TimerEventFilteringConfig(HRTIM_TypeDef *HRTIMx,
                                                      uint32_t TimerIdx,
                                                      uint32_t Event,
                                                      HRTIM_TimerEventFilteringCfgTypeDef * pTimerEventFilteringCfg);

void HRTIM_DeadTimeConfig(HRTIM_TypeDef *HRTIMx,
                                           uint32_t TimerIdx,
                                           HRTIM_DeadTimeCfgTypeDef* pDeadTimeCfg);

void HRTIM_ChopperModeConfig(HRTIM_TypeDef *HRTIMx,
                                              uint32_t TimerIdx,
                                              HRTIM_ChopperModeCfgTypeDef* pChopperModeCfg);

void HRTIM_BurstDMAConfig(HRTIM_TypeDef *HRTIMx,
                                           uint32_t TimerIdx,
                                           uint32_t RegistersToUpdate);

void HRTIM_SynchronizationConfig(HRTIM_TypeDef *HRTIMx,
                                                  HRTIM_SynchroCfgTypeDef * pSynchroCfg);

void HRTIM_BurstModeConfig(HRTIM_TypeDef *HRTIMx,
                                            HRTIM_BurstModeCfgTypeDef* pBurstModeCfg);

void HRTIM_EventConfig(HRTIM_TypeDef *HRTIMx,
                                        uint32_t Event,
                                        HRTIM_EventCfgTypeDef* pEventCfg);

void HRTIM_EventPrescalerConfig(HRTIM_TypeDef *HRTIMx,
                                                 uint32_t Prescaler);
 
void HRTIM_FaultConfig(HRTIM_TypeDef *hrtim,
                                        HRTIM_FaultCfgTypeDef* pFaultCfg,
                                        uint32_t Fault);

void HRTIM_FaultPrescalerConfig(HRTIM_TypeDef *HRTIMx,
                                                 uint32_t Prescaler);
void HRTIM_FaultModeCtl(HRTIM_TypeDef * HRTIMx, uint32_t Fault, uint32_t Enable);

void HRTIM_ADCTriggerConfig(HRTIM_TypeDef *HRTIMx,
                                             uint32_t ADCTrigger,
                                             HRTIM_ADCTriggerCfgTypeDef* pADCTriggerCfg);

void HRTIM_WaveformCounterStart(HRTIM_TypeDef *HRTIMx,
                                                 uint32_t TimersToStart);

void HRTIM_WaveformCounterStop(HRTIM_TypeDef *HRTIMx,
                                                 uint32_t TimersToStop);

void HRTIM_WaveformOutputStart(HRTIM_TypeDef *HRTIMx,
                                                uint32_t OutputsToStart);
void HRTIM_WaveformOutputStop(HRTIM_TypeDef * HRTIM_,
                                               uint32_t OutputsToStop);

void HRTIM_DLLCalibrationStart(HRTIM_TypeDef *HRTIMx,
                                                uint32_t CalibrationRate);
 
/* Interrupt/flags and DMA management */
void HRTIM_ITConfig(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, uint32_t HRTIM_TIM_IT, FunctionalState NewState);
void HRTIM_ITCommonConfig(HRTIM_TypeDef * HRTIMx, uint32_t HRTIM_CommonIT, FunctionalState NewState);

void HRTIM_ClearFlag(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, uint32_t HRTIM_FLAG);
void HRTIM_ClearCommonFlag(HRTIM_TypeDef * HRTIMx, uint32_t HRTIM_CommonFLAG);

void HRTIM_ClearITPendingBit(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, uint32_t HRTIM_IT);
void HRTIM_ClearCommonITPendingBit(HRTIM_TypeDef * HRTIMx, uint32_t HRTIM_CommonIT);

FlagStatus HRTIM_GetFlagStatus(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, uint32_t HRTIM_FLAG);
FlagStatus HRTIM_GetCommonFlagStatus(HRTIM_TypeDef * HRTIMx, uint32_t HRTIM_CommonFLAG);

ITStatus HRTIM_GetITStatus(HRTIM_TypeDef * HRTIMx, uint32_t TimerIdx, uint32_t HRTIM_IT);
ITStatus HRTIM_GetCommonITStatus(HRTIM_TypeDef * HRTIMx, uint32_t HRTIM_CommonIT);


void HRTIM_DMACmd(HRTIM_TypeDef* HRTIMx, uint32_t TimerIdx, uint32_t HRTIM_DMA, FunctionalState NewState);

void HRTIM_BurstModeCtl(HRTIM_TypeDef *HRTIMx,
                                         uint32_t Enable);

void HRTIM_SoftwareCapture(HRTIM_TypeDef *HRTIMx,
                                            uint32_t TimerIdx,
                                            uint32_t CaptureUnit);

void HRTIM_SoftwareUpdate(HRTIM_TypeDef *HRTIMx,
                                           uint32_t TimersToUpdate);

void HRTIM_SoftwareReset(HRTIM_TypeDef *HRTIMx,
                                          uint32_t TimersToReset);


uint32_t HRTIM_GetCapturedValue(HRTIM_TypeDef *HRTIMx,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit);

void HRTIM_WaveformOutputConfig(HRTIM_TypeDef * HRTIMx,
                                                uint32_t TimerIdx,
                                                uint32_t Output,
                                                HRTIM_OutputCfgTypeDef * pOutputCfg);

void HRTIM_WaveformSetOutputLevel(HRTIM_TypeDef *HRTIMx,
                                                   uint32_t TimerIdx,
                                                   uint32_t Output, 
                                                   uint32_t OutputLevel);

uint32_t HRTIM_WaveformGetOutputLevel(HRTIM_TypeDef *HRTIMx,
                                          uint32_t TimerIdx,
                                          uint32_t Output);

uint32_t HRTIM_WaveformGetOutputState(HRTIM_TypeDef * HRTIMx,
                                          uint32_t TimerIdx,
                                          uint32_t Output);
uint32_t HRTIM_GetDelayedProtectionStatus(HRTIM_TypeDef *HRTIMx,
                                              uint32_t TimerIdx,
                                              uint32_t Output);

uint32_t HRTIM_GetBurstStatus(HRTIM_TypeDef *HRTIMx);

uint32_t HRTIM_GetCurrentPushPullStatus(HRTIM_TypeDef *HRTIMx,
                                            uint32_t TimerIdx);

uint32_t HRTIM_GetIdlePushPullStatus(HRTIM_TypeDef *HRTIMx,
                                         uint32_t TimerIdx);


#ifdef __cplusplus
}}
#endif
