/*
 * stm32pwr.h
 *
 *  Created on: 24-07-2012
 *      Author: lucck
 */

#pragma once

#include <stm32lib.h>

#ifdef __cplusplus
 namespace stm32 {
#endif


#ifdef _PWR_TYPEDEF_HAS_CR2_
#define PWR_CR_REG PWR->CR1
#define PWR_CSR_REG PWR->CSR1
#else
#define PWR_CR_REG PWR->CR
#define PWR_CSR_REG PWR->CSR
#endif

 /* Alias word address of DBP bit */
#define DBP_BitNumber            0x08
/* Alias word address of PVDE bit */
#define PVDE_BitNumber           0x04
/* Alias word address of FPDS bit */
#define FPDS_BitNumber           0x09

#ifdef STM32MCU_MAJOR_TYPE_F4
 /* Alias word address of PMODE bit */
#define PMODE_BitNumber           0x0E
#endif



 /* CR register bit mask */
#define CR_DS_MASK               ((uint32_t)0xFFFFFFFC)
#define CR_PLS_MASK              ((uint32_t)0xFFFFFF1F)
 /* Alias word address of EWUP bit */
#define EWUP_BitNumber           0x08
/* Alias word address of BRE bit */
#define BRE_BitNumber            0x09


 /**
   * @brief  Enables or disables access to the backup domain (RTC registers, RTC
   *         backup data registers and backup SRAM).
   * @note   If the HSE divided by 2, 3, ..31 is used as the RTC clock, the
   *         Backup Domain Access should be kept enabled.
   * @param  NewState: new state of the access to the backup domain.
   *          This parameter can be: ENABLE or DISABLE.
   * @retval None
   */
static inline void pwr_backup_access_cmd(bool enable)
{
	if(enable) PWR_CR_REG |= 1U<<DBP_BitNumber;
	else PWR_CR_REG &= ~(1U<<DBP_BitNumber);
}

/**
  * @brief  Configures the voltage threshold detected by the Power Voltage Detector(PVD).
  * @param  PWR_PVDLevel: specifies the PVD detection level
  *          This parameter can be one of the following values:
  *            @arg PWR_PVDLevel_0
  *            @arg PWR_PVDLevel_1
  *            @arg PWR_PVDLevel_2
  *            @arg PWR_PVDLevel_3
  *            @arg PWR_PVDLevel_4
  *            @arg PWR_PVDLevel_5
  *            @arg PWR_PVDLevel_6
  *            @arg PWR_PVDLevel_7
  * @note   Refer to the electrical characteristics of your device datasheet for
  *         more details about the voltage threshold corresponding to each
  *         detection level.
  * @retval None
  */
static inline void pwr_pvd_level_config(uint32_t PWR_PVDLevel)
{
  uint32_t tmpreg = 0;

  tmpreg = PWR_CR_REG;

  /* Clear PLS[7:5] bits */
  tmpreg &= CR_PLS_MASK;

  /* Set PLS[7:5] bits according to PWR_PVDLevel value */
  tmpreg |= PWR_PVDLevel;

  /* Store the new value */
  PWR_CR_REG = tmpreg;
}

/**
  * @brief  Enables or disables the Power Voltage Detector(PVD).
  * @param  NewState: new state of the PVD.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_pvd_cmd(bool enable)
{

  if(enable) PWR_CR_REG |= 1U<<PVDE_BitNumber;
  else PWR_CR_REG &= ~(1U<<PVDE_BitNumber);
}

/**
  * @brief  Enables or disables the WakeUp Pin functionality.
  * @param  NewState: new state of the WakeUp Pin functionality.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
#ifndef PWR_WakeUpPin_2
static inline void pwr_wake_up_pin_cmd(bool enable)
{

	if(enable) PWR_CSR_REG |=  1U<<EWUP_BitNumber;
	else PWR_CSR_REG &=  ~(1U<<EWUP_BitNumber);
}
#else

static inline void pwr_wake_up_pin_cmd(uint32_t PWR_WakeUpPin, bool en )
{
  if ( en )
  {
    /* Enable the EWUPx pin */
    PWR->CSR |= PWR_WakeUpPin;
  }
  else
  {
    /* Disable the EWUPx pin */
    PWR->CSR &= ~PWR_WakeUpPin;
  }
}
#endif





/**
  * @brief  Enables or disables the WakeUp Pin functionality.
  * @param  NewState: new state of the WakeUp Pin functionality.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_backup_regulator_cmd( bool en )
{
	if( en )
		PWR->CSR |=  1U<<9U;
	else
		PWR->CSR &= ~(1U<<9U);
}

#ifdef STM32MCU_MAJOR_TYPE_F4
/**
  * @brief  Configures the main internal regulator output voltage.
  * @param  PWR_Regulator_Voltage: specifies the regulator output voltage to achieve
  *         a tradeoff between performance and power consumption when the device does
  *         not operate at the maximum frequency (refer to the datasheets for more details).
  *          This parameter can be one of the following values:
  *            @arg PWR_Regulator_Voltage_Scale1: Regulator voltage output Scale 1 mode,
  *                                                System frequency up to 168 MHz.
  *            @arg PWR_Regulator_Voltage_Scale2: Regulator voltage output Scale 2 mode,
  *                                                System frequency up to 144 MHz.
  * @retval None
  */
static inline void pwr_main_regulator_mode_config(uint32_t PWR_Regulator_Voltage)
{
  if (PWR_Regulator_Voltage == PWR_Regulator_Voltage_Scale2)
  {
    PWR->CR &= ~PWR_Regulator_Voltage_Scale1;
  }
  else
  {
    PWR->CR |= PWR_Regulator_Voltage_Scale1;
  }
}
#endif

#if defined(STM32MCU_MAJOR_TYPE_F37)
/**
  * @brief  Enables or disables the WakeUp Pin functionality.
  * @param  PWR_SDADCAnalog: specifies the SDADC.
  *          This parameter can be: PWR_SDADCAnalog_1, PWR_SDADCAnalog_2 or PWR_SDADCAnalog_3.
  * @param  NewState: new state of the SDADC Analog functionality.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_sdadc_analog_cmd(uint32_t PWR_SDADCAnalog, bool en )
{
  if ( en )
  {
    /* Enable the SDADCx analog */
    PWR->CR |= PWR_SDADCAnalog;
  }
  else
  {
    /* Disable the SDADCx analog */
    PWR->CR &= ~PWR_SDADCAnalog;
  }
}
#endif


#if defined(STM32MCU_MAJOR_TYPE_F2) || defined(STM32MCU_MAJOR_TYPE_F4 )
/**
  * @brief  Enables or disables the Flash Power Down in STOP mode.
  * @param  NewState: new state of the Flash power mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_flash_power_down_cmd(bool enable)
{
	if(enable) PWR->CR = 1U<<FPDS_BitNumber;
	else PWR->CR = ~(1U<<FPDS_BitNumber);
}
#endif


/**
  * @brief  Enters STOP mode.
  *
  * @note   In Stop mode, all I/O pins keep the same state as in Run mode.
  * @note   When exiting Stop mode by issuing an interrupt or a wakeup event,
  *         the HSI RC oscillator is selected as system clock.
  * @note   When the voltage regulator operates in low power mode, an additional
  *         startup delay is incurred when waking up from Stop mode.
  *         By keeping the internal regulator ON during Stop mode, the consumption
  *         is higher although the startup time is reduced.
  *
  * @param  PWR_Regulator: specifies the regulator state in STOP mode.
  *          This parameter can be one of the following values:
  *            @arg PWR_Regulator_ON: STOP mode with regulator ON
  *            @arg PWR_Regulator_LowPower: STOP mode with regulator in low power mode
  * @param  PWR_STOPEntry: specifies if STOP mode in entered with WFI or WFE instruction.
  *          This parameter can be one of the following values:
  *            @arg PWR_STOPEntry_WFI: enter STOP mode with WFI instruction
  *            @arg PWR_STOPEntry_WFE: enter STOP mode with WFE instruction
  * @retval None
  */
#ifndef SCB_SCR_SLEEPDEEP_Msk
#define SCB_SCR_SLEEPDEEP_Msk SCB_SCR_SLEEPDEEP
#endif

#ifndef PWR_STOPEntry_WFI
#	define PWR_STOPEntry_WFI               1
#endif
#ifndef PWR_STOPEntry_WFE
#	define PWR_STOPEntry_WFE               2
#endif

static inline void pwr_enter_stop_mode(uint32_t PWR_Regulator, uint8_t PWR_STOPEntry)
{
  uint32_t tmpreg = 0;

  /* Select the regulator state in STOP mode ---------------------------------*/
  tmpreg = PWR_CR_REG;
  /* Clear PDDS and LPDSR bits */
  tmpreg &= CR_DS_MASK;

  /* Set LPDSR bit according to PWR_Regulator value */
  tmpreg |= PWR_Regulator;

  /* Store the new value */
  PWR_CR_REG = tmpreg;

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /* Select STOP mode entry --------------------------------------------------*/
  if(PWR_STOPEntry == PWR_STOPEntry_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
    __WFE();
  }
  /* Reset SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP_Msk);
}

#ifdef PWR_CR_CWUF
/**
  * @brief  Enters STANDBY mode.
  * @note   In Standby mode, all I/O pins are high impedance except for:
  *          - Reset pad (still available)
  *          - RTC_AF1 pin (PC13) if configured for tamper, time-stamp, RTC
  *            Alarm out, or RTC clock calibration out.
  *          - RTC_AF2 pin (PI8) if configured for tamper or time-stamp.
  *          - WKUP pin 1 (PA0) if enabled.
  * @param  None
  * @retval None
  */
static inline void pwr_enter_standby_mode(void)
{
  /* Clear Wakeup flag */
  PWR_CR_REG |= PWR_CR_CWUF;

  /* Select STANDBY mode */
  PWR_CR_REG |= PWR_CR_PDDS;

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /* Request Wait For Interrupt */
  __WFI();
}
#endif

#ifdef PWR_SLEEPEntry_WFI
/**
  * @brief  Enters Sleep mode.
  * @note   In Sleep mode, all I/O pins keep the same state as in Run mode.
  * @param  PWR_SLEEPEntry: specifies if SLEEP mode in entered with WFI or WFE instruction.
  *          This parameter can be one of the following values:
  *             @arg PWR_SLEEPEntry_WFI: enter SLEEP mode with WFI instruction
  *             @arg PWR_SLEEPEntry_WFE: enter SLEEP mode with WFE instruction
  * @retval None
  */
static inline void pwr_enter_sleep_mode(uint8_t PWR_SLEEPEntry)
{

  /* Clear SLEEPDEEP bit of Cortex-M4 System Control Register */
  SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP_Msk);

  /* Select SLEEP mode entry -------------------------------------------------*/
  if(PWR_SLEEPEntry == PWR_SLEEPEntry_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
    __WFE();
  }
}
#endif


/**
  * @brief  Checks whether the specified PWR flag is set or not.
  * @param  PWR_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg PWR_FLAG_WU: Wake Up flag. This flag indicates that a wakeup event
  *                  was received from the WKUP pin or from the RTC alarm (Alarm A
  *                  or Alarm B), RTC Tamper event, RTC TimeStamp event or RTC Wakeup.
  *                  An additional wakeup event is detected if the WKUP pin is enabled
  *                  (by setting the EWUP bit) when the WKUP pin level is already high.
  *            @arg PWR_FLAG_SB: StandBy flag. This flag indicates that the system was
  *                  resumed from StandBy mode.
  *            @arg PWR_FLAG_PVDO: PVD Output. This flag is valid only if PVD is enabled
  *                  by the PWR_PVDCmd() function. The PVD is stopped by Standby mode
  *                  For this reason, this bit is equal to 0 after Standby or reset
  *                  until the PVDE bit is set.
  *            @arg PWR_FLAG_BRR: Backup regulator ready flag. This bit is not reset
  *                  when the device wakes up from Standby mode or by a system reset
  *                  or power reset.
  *            @arg PWR_FLAG_VOSRDY: This flag indicates that the Regulator voltage
  *                 scaling output selection is ready.
  * @retval The new state of PWR_FLAG (SET or RESET).
  */
static inline bool pwr_get_flag_status(uint32_t PWR_FLAG)
{

  return ((PWR_CSR_REG & PWR_FLAG) != (uint32_t)0);
}

/**
  * @brief  Clears the PWR's pending flags.
  * @param  PWR_FLAG: specifies the flag to clear.
  *          This parameter can be one of the following values:
  *            @arg PWR_FLAG_WU: Wake Up flag
  *            @arg PWR_FLAG_SB: StandBy flag
  * @retval None
  */
static inline void pwr_clear_flag(uint32_t PWR_FLAG)
{

  PWR_CR_REG |=  PWR_FLAG << 2;
}


#undef DBP_BitNumber
#undef PVDE_BitNumber
#undef FPDS_BitNumber
#undef PMODE_BitNumber
#undef CR_DS_MASK
#undef CR_PLS_MASK
#undef EWUP_BitNumber
#undef BRE_BitNumber


 #ifdef __cplusplus
}
#endif


