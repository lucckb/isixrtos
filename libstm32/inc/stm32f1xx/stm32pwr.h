#ifndef STM32F1_PWR_H_
#define STM32F1_PWR_H_
/* Based on the stm32 lib
 * Lucjan Bryndza <lucck@boff.pl>
 *
 */
/* ---------------------------------------------------------------------------- */
#include "stm32f10x_lib.h"
#include <stddef.h>
#include "stm32f10x_pwr.h"

/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
 namespace stm32 {
#endif
 /* ---------------------------------------------------------------------------- */

#ifdef __cplusplus
namespace _internal {
namespace pwr {


/* --------- PWR registers bit address in the alias region ---------- */
static const unsigned PWR_OFFSET      =         PWR_BASE - PERIPH_BASE;

/* --- CR Register ---*/

/* Alias word address of DBP bit */
static const unsigned  CR_OFFSET      =         PWR_OFFSET + 0x00;
static const unsigned  DBP_BitNumber   =         0x08;
static const unsigned  CR_DBP_BB       =        PERIPH_BB_BASE + (CR_OFFSET * 32) + (DBP_BitNumber * 4);

/* Alias word address of PVDE bit */
static const unsigned  PVDE_BitNumber  =         0x04;
static const unsigned  CR_PVDE_BB      =        PERIPH_BB_BASE + (CR_OFFSET * 32) + (PVDE_BitNumber * 4);

/* --- CSR Register ---*/

/* Alias word address of EWUP bit */
static const unsigned  CSR_OFFSET     =          PWR_OFFSET + 0x04;
static const unsigned  EWUP_BitNumber   =        0x08;
static const unsigned  CSR_EWUP_BB     =         PERIPH_BB_BASE + (CSR_OFFSET * 32) + (EWUP_BitNumber * 4);

/* ------------------ PWR registers bit mask ------------------------ */

/* CR register bit mask */
static const unsigned  CR_DS_MASK   =           0xFFFFFFFC;
static const unsigned  CR_PLS_MASK  =           0xFFFFFF1F;
}}

#else /* __cplusplus */

#define PWR_OFFSET              (PWR_BASE - PERIPH_BASE)
/* Alias word address of DBP bit */
#define   CR_OFFSET             (PWR_OFFSET + 0x00)
#define   DBP_BitNumber         0x08
#define   CR_DBP_BB             (PERIPH_BB_BASE + (CR_OFFSET * 32) + (DBP_BitNumber * 4))

/* Alias word address of PVDE bit */
#define   PVDE_BitNumber      0x04
#define   CR_PVDE_BB           (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PVDE_BitNumber * 4))

/* --- CSR Register ---*/

/* Alias word address of EWUP bit */
#define   CSR_OFFSET          (PWR_OFFSET + 0x04)
#define   EWUP_BitNumber       0x08
#define   CSR_EWUP_BB          (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (EWUP_BitNumber * 4))

/* ------------------ PWR registers bit mask ------------------------ */

/* CR register bit mask */
#define  CR_DS_MASK            0xFFFFFFFC
#define  CR_PLS_MASK           0xFFFFFF1F

#endif /* __cplusplus */
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables access to the RTC and backup registers.
  * @param  NewState: new state of the access to the RTC and backup registers.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_backup_access_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::pwr;
#endif
	*(__IO uint32_t *) CR_DBP_BB = (uint32_t)enable;
}

/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the Power Voltage Detector(PVD).
  * @param  NewState: new state of the PVD.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_pvd_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::pwr;
#endif
  /* Check the parameters */
  *(__IO uint32_t *) CR_PVDE_BB = (uint32_t)enable;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures the voltage threshold detected by the Power Voltage Detector(PVD).
  * @param  PWR_PVDLevel: specifies the PVD detection level
  *   This parameter can be one of the following values:
  *     @arg PWR_PVDLevel_2V2: PVD detection level set to 2.2V
  *     @arg PWR_PVDLevel_2V3: PVD detection level set to 2.3V
  *     @arg PWR_PVDLevel_2V4: PVD detection level set to 2.4V
  *     @arg PWR_PVDLevel_2V5: PVD detection level set to 2.5V
  *     @arg PWR_PVDLevel_2V6: PVD detection level set to 2.6V
  *     @arg PWR_PVDLevel_2V7: PVD detection level set to 2.7V
  *     @arg PWR_PVDLevel_2V8: PVD detection level set to 2.8V
  *     @arg PWR_PVDLevel_2V9: PVD detection level set to 2.9V
  * @retval None
  */
static inline void pwr_pvd_level_config(uint32_t PWR_PVDLevel)
{
#ifdef __cplusplus
	using namespace _internal::pwr;
#endif
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
  * @brief  Enables or disables the WakeUp Pin functionality.
  * @param  NewState: new state of the WakeUp Pin functionality.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void pwr_wakeup_pin_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::pwr;
#endif
  /* Check the parameters */
  *(__IO uint32_t *) CSR_EWUP_BB = (uint32_t)enable;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enters STOP mode.
  * @param  PWR_Regulator: specifies the regulator state in STOP mode.
  *   This parameter can be one of the following values:
  *     @arg PWR_Regulator_ON: STOP mode with regulator ON
  *     @arg PWR_Regulator_LowPower: STOP mode with regulator in low power mode
  * @param  PWR_STOPEntry: specifies if STOP mode in entered with WFI or WFE instruction.
  *   This parameter can be one of the following values:
  *     @arg PWR_STOPEntry_WFI: enter STOP mode with WFI instruction
  *     @arg PWR_STOPEntry_WFE: enter STOP mode with WFE instruction
  * @retval None
  */
static inline void pwr_enter_stop_mode(uint32_t PWR_Regulator, uint8_t PWR_STOPEntry)
{
#ifdef __cplusplus
	using namespace _internal::pwr;
#endif
  uint32_t tmpreg = 0;
  /* Check the parameters */
  /* Select the regulator state in STOP mode ---------------------------------*/
  tmpreg = PWR->CR;
  /* Clear PDDS and LPDS bits */
  tmpreg &= CR_DS_MASK;
  /* Set LPDS bit according to PWR_Regulator value */
  tmpreg |= PWR_Regulator;
  /* Store the new value */
  PWR->CR = tmpreg;
  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP;
  
  /* Select STOP mode entry --------------------------------------------------*/
  if(PWR_STOPEntry == PWR_STOPEntry_WFI)
  {   
    /* Request Wait For Interrupt */
    asm volatile("wfi\n");
  }
  else
  {
    /* Request Wait For Event */
	  asm volatile("wfe\n");
  }
  
  /* Reset SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP);  
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enters STANDBY mode.
  * @param  None
  * @retval None
  */
static inline void pwr_enter_standby_mode(void)
{
  /* Clear Wake-up flag */
  PWR->CR |= PWR_CR_CWUF;
  /* Select STANDBY mode */
  PWR->CR |= PWR_CR_PDDS;
  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP;
  /* Request Wait For Interrupt */
  asm volatile("wfi\n");
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the specified PWR flag is set or not.
  * @param  PWR_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg PWR_FLAG_WU: Wake Up flag
  *     @arg PWR_FLAG_SB: StandBy flag
  *     @arg PWR_FLAG_PVDO: PVD Output
  * @retval The new state of PWR_FLAG (SET or RESET).
  */
static inline bool pwr_get_flag_status(uint32_t PWR_FLAG)
{
	return (PWR->CSR & PWR_FLAG)?true:false;
}

/* ---------------------------------------------------------------------------- */
/**
  * @brief  Clears the PWR's pending flags.
  * @param  PWR_FLAG: specifies the flag to clear.
  *   This parameter can be one of the following values:
  *     @arg PWR_FLAG_WU: Wake Up flag
  *     @arg PWR_FLAG_SB: StandBy flag
  * @retval None
  */
static inline void pwr_clear_flag(uint32_t PWR_FLAG)
{
  PWR->CR |=  PWR_FLAG << 2;
}
/* ---------------------------------------------------------------------------- */

#ifdef __cplusplus
 }

#else	//Undefine local #define

#undef PWR_OFFSET
#undef CR_OFFSET
#undef DBP_BitNumber
#undef CR_DBP_BB
#undef  PVDE_BitNumber
#undef CR_PVDE_BB
#undef CSR_OFFSET
#undef EWUP_BitNumber
#undef CSR_EWUP_BB
#undef CR_DS_MASK
#undef CR_PLS_MASK

#endif /*__cplusplus*/

#endif /* STM32F1_PWR_H_ */
/* ---------------------------------------------------------------------------- */
