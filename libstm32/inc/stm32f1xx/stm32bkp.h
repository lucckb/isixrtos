/*
 * stm32bkp.h
 *
 *  Created on: 13-02-2012
 *      Author: lucck
 */
/* ---------------------------------------------------------------------------- */
#ifndef STM32F1BKP_H_
#define STM32F1BKP_H_
/* ---------------------------------------------------------------------------- */
#include "stm32f10x.h"
#include <stddef.h>
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
 namespace stm32 {
#endif
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
namespace _internal {
namespace bkp {

static const unsigned long BKP_OFFSET    =    BKP_BASE - PERIPH_BASE;
/* Alias word address of TPAL bit */
static const unsigned long CR_OFFSET     =    BKP_OFFSET + 0x30;
static const int TPAL_BitNumber  =  0x01;
static const unsigned long CR_TPAL_BB   =     PERIPH_BB_BASE + (CR_OFFSET * 32) + (TPAL_BitNumber * 4);
/* Alias word address of TPE bit */
static const int TPE_BitNumber  =   0x00;
static const unsigned long CR_TPE_BB     =    PERIPH_BB_BASE + (CR_OFFSET * 32) + (TPE_BitNumber * 4);
/* Alias word address of TPIE bit */
static const unsigned long CSR_OFFSET  =      BKP_OFFSET + 0x34;
static const int TPIE_BitNumber  =  0x02;
static const unsigned long CSR_TPIE_BB   =    PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TPIE_BitNumber * 4);
/* Alias word address of TIF bit */
static const int TIF_BitNumber  =   0x09;
static const unsigned long CSR_TIF_BB    =    PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TIF_BitNumber * 4);
/* Alias word address of TEF bit */
static const int TEF_BitNumber  =   0x08;
static const unsigned long CSR_TEF_BB     =   PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TEF_BitNumber * 4);
/* RTCCR register bit mask */
static const uint16_t RTCCR_CAL_MASK   = 0xFF80;
static const uint16_t RTCCR_MASK      =  0xFC7F;

}}
#else /* __cplusplus */

#define BKP_OFFSET        (BKP_BASE - PERIPH_BASE)
/* Alias word address of TPAL bit */
#define CR_OFFSET         (BKP_OFFSET + 0x30)
#define TPAL_BitNumber    0x01
#define CR_TPAL_BB        (PERIPH_BB_BASE + (CR_OFFSET * 32) + (TPAL_BitNumber * 4))
/* Alias word address of TPE bit */
#define TPE_BitNumber     0x00
#define CR_TPE_BB         (PERIPH_BB_BASE + (CR_OFFSET * 32) + (TPE_BitNumber * 4))
/* Alias word address of TPIE bit */
#define CSR_OFFSET        (BKP_OFFSET + 0x34)
#define TPIE_BitNumber    0x02
#define CSR_TPIE_BB       (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TPIE_BitNumber * 4))
/* Alias word address of TIF bit */
#define TIF_BitNumber     0x09
#define CSR_TIF_BB        (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TIF_BitNumber * 4))
/* Alias word address of TEF bit */
#define TEF_BitNumber     0x08
#define CSR_TEF_BB        (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TEF_BitNumber * 4))
/* RTCCR register bit mask */
#define RTCCR_CAL_MASK    ((uint16_t)0xFF80)
#define RTCCR_MASK        ((uint16_t)0xFC7F)

#endif /* __cplusplus */
/* ---------------------------------------------------------------------------- */
enum  BKP_DR_REGS
{
	BKP_DR1        =                   0x0004,
	BKP_DR2        =                   0x0008,
	BKP_DR3        =                   0x000C,
	BKP_DR4        =                   0x0010,
	BKP_DR5        =                   0x0014,
	BKP_DR6        =                   0x0018,
	BKP_DR7        =                   0x001C,
	BKP_DR8        =                   0x0020,
	BKP_DR9        =                   0x0024,
	BKP_DR10       =                   0x0028,
	BKP_DR11       =                   0x0040,
	BKP_DR12       =                   0x0044,
	BKP_DR13       =                   0x0048,
	BKP_DR14       =                   0x004C,
	BKP_DR15       =                   0x0050,
	BKP_DR16       =                   0x0054,
	BKP_DR17       =                   0x0058,
	BKP_DR18       =                   0x005C,
	BKP_DR19       =                   0x0060,
	BKP_DR20       =                   0x0064,
	BKP_DR21       =                   0x0068,
	BKP_DR22       =                   0x006C,
	BKP_DR23       =                   0x0070,
	BKP_DR24       =                   0x0074,
	BKP_DR25       =                   0x0078,
	BKP_DR26       =                   0x007C,
	BKP_DR27       =                   0x0080,
	BKP_DR28       =                   0x0084,
	BKP_DR29       =                   0x0088,
	BKP_DR30       =                   0x008C,
	BKP_DR31       =                   0x0090,
	BKP_DR32       =                   0x0094,
	BKP_DR33       =                   0x0098,
	BKP_DR34       =                   0x009C,
	BKP_DR35       =                   0x00A0,
	BKP_DR36       =                   0x00A4,
	BKP_DR37       =                   0x00A8,
	BKP_DR38       =                   0x00AC,
	BKP_DR39       =                   0x00B0,
	BKP_DR40       =                   0x00B4,
	BKP_DR41       =                   0x00B8,
	BKP_DR42       =                   0x00BC
};


/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures the Tamper Pin active level.
  * @param  BKP_TamperPinLevel: specifies the Tamper Pin active level.
  *   This parameter can be one of the following values:
  *     @arg BKP_TamperPinLevel_High: Tamper pin active on high level
  *     @arg BKP_TamperPinLevel_Low: Tamper pin active on low level
  * @retval None
  */
static inline void bkp_tamper_pin_level_config(uint16_t BKP_TamperPinLevel)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  /* Check the parameters */
  *(__IO uint32_t *) CR_TPAL_BB = BKP_TamperPinLevel;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the Tamper Pin activation.
  * @param  NewState: new state of the Tamper Pin activation.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void bkp_tamper_pin_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  *(__IO uint32_t *) CR_TPE_BB = (uint32_t)enable;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the Tamper Pin Interrupt.
  * @param  NewState: new state of the Tamper Pin Interrupt.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void bkp_it_config(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  *(__IO uint32_t *) CSR_TPIE_BB = (uint32_t)enable;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Select the RTC output source to output on the Tamper pin.
  * @param  BKP_RTCOutputSource: specifies the RTC output source.
  *   This parameter can be one of the following values:
  *     @arg BKP_RTCOutputSource_None: no RTC output on the Tamper pin.
  *     @arg BKP_RTCOutputSource_CalibClock: output the RTC clock with frequency
  *                                          divided by 64 on the Tamper pin.
  *     @arg BKP_RTCOutputSource_Alarm: output the RTC Alarm pulse signal on
  *                                     the Tamper pin.
  *     @arg BKP_RTCOutputSource_Second: output the RTC Second pulse signal on
  *                                      the Tamper pin.
  * @retval None
  */
static inline void bkp_rtc_output_config(uint16_t BKP_RTCOutputSource)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  uint16_t tmpreg = 0;
  tmpreg = BKP->RTCCR;
  /* Clear CCO, ASOE and ASOS bits */
  tmpreg &= RTCCR_MASK;

  /* Set CCO, ASOE and ASOS bits according to BKP_RTCOutputSource value */
  tmpreg |= BKP_RTCOutputSource;
  /* Store the new value */
  BKP->RTCCR = tmpreg;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Sets RTC Clock Calibration value.
  * @param  CalibrationValue: specifies the RTC Clock Calibration value.
  *   This parameter must be a number between 0 and 0x7F.
  * @retval None
  */
static inline void bkp_set_rtc_calibration_value(uint8_t CalibrationValue)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  uint16_t tmpreg = 0;
  tmpreg = BKP->RTCCR;
  /* Clear CAL[6:0] bits */
  tmpreg &= RTCCR_CAL_MASK;
  /* Set CAL[6:0] bits according to CalibrationValue value */
  tmpreg |= CalibrationValue;
  /* Store the new value */
  BKP->RTCCR = tmpreg;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Writes user data to the specified Data Backup Register.
  * @param  BKP_DR: specifies the Data Backup Register.
  *   This parameter can be BKP_DRx where x:[1, 42]
  * @param  Data: data to write
  * @retval None
  */
static inline void bkp_write_backup_register(uint16_t BKP_DR, uint16_t Data)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  __IO uint32_t tmp = 0;

  tmp = (uint32_t)BKP_BASE;
  tmp += BKP_DR;

  *(__IO uint32_t *) tmp = Data;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Reads data from the specified Data Backup Register.
  * @param  BKP_DR: specifies the Data Backup Register.
  *   This parameter can be BKP_DRx where x:[1, 42]
  * @retval The content of the specified Data Backup Register
  */
static inline uint16_t bkp_read_backup_register(uint16_t BKP_DR)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  __IO uint32_t tmp = 0;

  tmp = (uint32_t)BKP_BASE;
  tmp += BKP_DR;

  return (*(__IO uint16_t *) tmp);
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the Tamper Pin Event flag is set or not.
  * @param  None
  * @retval The new state of the Tamper Pin Event flag (SET or RESET).
  */
static inline bool bkp_get_flag_status(void)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  return *(__IO uint32_t *) CSR_TEF_BB;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Clears Tamper Pin Event pending flag.
  * @param  None
  * @retval None
  */
static inline void bkp_clear_flag(void)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  /* Set CTE bit to clear Tamper Pin Event flag */
  BKP->CSR |= BKP_CSR_CTE;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the Tamper Pin Interrupt has occurred or not.
  * @param  None
  * @retval The new state of the Tamper Pin Interrupt (SET or RESET).
  */
static inline bool bkp_get_it_status(void)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  return*(__IO uint32_t *) CSR_TIF_BB;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Clears Tamper Pin Interrupt pending bit.
  * @param  None
  * @retval None
  */
static inline void bkp_clear_it_pending_bit(void)
{
#ifdef __cplusplus
	using namespace _internal::bkp;
#endif
  /* Set CTI bit to clear Tamper Pin Interrupt pending bit */
  BKP->CSR |= BKP_CSR_CTI;
}
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
}

#else	//Undefine local #define

#undef BKP_OFFSET
#undef CR_OFFSET
#undef TPAL_BitNumber
#undef CR_TPAL_BB
#undef TPE_BitNumber
#undef CR_TPE_BB
#undef CSR_OFFSET
#undef TPIE_BitNumber
#undef CSR_TPIE_BB
#undef TIF_BitNumber
#undef CSR_TIF_BB
#undef TEF_BitNumber
#undef CSR_TEF_BB
#undef RTCCR_CAL_MASK
#undef RTCCR_MASK

#endif /*__cplusplus*/
/* ---------------------------------------------------------------------------- */
#endif /* STM32BKP_H_ */
/* ---------------------------------------------------------------------------- */
