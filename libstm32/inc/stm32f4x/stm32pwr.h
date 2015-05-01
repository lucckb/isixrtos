/*
 * stm32pwr.h
 *
 *  Created on: 24-07-2012
 *      Author: lucck
 */

#pragma once

#include <stm32lib.h>
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
 namespace stm32 {
#endif
/* ---------------------------------------------------------------------------- */
 /* Alias word address of DBP bit */
#define PWR_OFFSET               (PWR_BASE - PERIPH_BASE)
#define CR_OFFSET                (PWR_OFFSET + 0x00)
#define DBP_BitNumber            0x08
#define CR_DBP_BB                (PERIPH_BB_BASE + (CR_OFFSET * 32) + (DBP_BitNumber * 4))

/* Alias word address of PVDE bit */
#define PVDE_BitNumber           0x04
#define CR_PVDE_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PVDE_BitNumber * 4))

/* Alias word address of FPDS bit */
#define FPDS_BitNumber           0x09
#define CR_FPDS_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (FPDS_BitNumber * 4))

#ifdef STM32MCU_MAJOR_TYPE_F4
 /* Alias word address of PMODE bit */
#define PMODE_BitNumber           0x0E
#define CR_PMODE_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PMODE_BitNumber * 4))
#endif
 /* CR register bit mask */
#define CR_DS_MASK               ((uint32_t)0xFFFFFFFC)
#define CR_PLS_MASK              ((uint32_t)0xFFFFFF1F)
 /* Alias word address of EWUP bit */
#define CSR_OFFSET               (PWR_OFFSET + 0x04)
#define EWUP_BitNumber           0x08
#define CSR_EWUP_BB              (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (EWUP_BitNumber * 4))
/* Alias word address of BRE bit */
#define BRE_BitNumber            0x09
#define CSR_BRE_BB              (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (BRE_BitNumber * 4))

/* ---------------------------------------------------------------------------- */
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

   *(__IO uint32_t *) CR_DBP_BB = (uint32_t)enable;
}
/* ---------------------------------------------------------------------------- */
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

  tmpreg = PWR->CR;

  /* Clear PLS[7:5] bits */
  tmpreg &= CR_PLS_MASK;

  /* Set PLS[7:5] bits according to PWR_PVDLevel value */
  tmpreg |= PWR_PVDLevel;

  /* Store the new value */
  PWR->CR = tmpreg;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the Power Voltage Detector(PVD).
  * @param  NewState: new state of the PVD.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_pvd_cmd(bool enable)
{

  *(__IO uint32_t *) CR_PVDE_BB = (uint32_t)enable;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the WakeUp Pin functionality.
  * @param  NewState: new state of the WakeUp Pin functionality.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_wake_up_pin_cmd(bool enable)
{

  *(__IO uint32_t *) CSR_EWUP_BB = (uint32_t)enable;
}
/* ---------------------------------------------------------------------------- */
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
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the Flash Power Down in STOP mode.
  * @param  NewState: new state of the Flash power mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_flash_power_down_cmd(bool enable)
{
  *(__IO uint32_t *) CR_FPDS_BB = (uint32_t)enable;
}

/* ---------------------------------------------------------------------------- */
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
static inline void pwr_enter_stop_mode(uint32_t PWR_Regulator, uint8_t PWR_STOPEntry)
{
  uint32_t tmpreg = 0;

  /* Select the regulator state in STOP mode ---------------------------------*/
  tmpreg = PWR->CR;
  /* Clear PDDS and LPDSR bits */
  tmpreg &= CR_DS_MASK;

  /* Set LPDSR bit according to PWR_Regulator value */
  tmpreg |= PWR_Regulator;

  /* Store the new value */
  PWR->CR = tmpreg;

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
/* ---------------------------------------------------------------------------- */
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
  PWR->CR |= PWR_CR_CWUF;

  /* Select STANDBY mode */
  PWR->CR |= PWR_CR_PDDS;

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /* Request Wait For Interrupt */
  __WFI();
}
/* ---------------------------------------------------------------------------- */
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

  return ((PWR->CSR & PWR_FLAG) != (uint32_t)0);
}
/* ---------------------------------------------------------------------------- */
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

  PWR->CR |=  PWR_FLAG << 2;
}
/* ---------------------------------------------------------------------------- */

#undef CR_OFFSET
#undef DBP_BitNumber
#undef CR_DBP_BB
#undef PVDE_BitNumber
#undef CR_PVDE_BB
#undef FPDS_BitNumber
#undef CR_FPDS_BB
#undef PMODE_BitNumber
#undef CR_PMODE_BB
#undef PWR_OFFSET
#undef CR_DS_MASK
#undef CR_PLS_MASK
#undef CSR_OFFSET
#undef EWUP_BitNumber
#undef CSR_EWUP_BB
#undef BRE_BitNumber
#undef CSR_BRE_BB

/* ---------------------------------------------------------------------------- */
 #ifdef __cplusplus
}
#endif
/* ---------------------------------------------------------------------------- */

