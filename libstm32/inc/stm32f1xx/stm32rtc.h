#pragma once

#include "stm32f10x_lib.h"
#include "stm32f10x_rtc.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


#define RTC_LSB_MASK     ((uint32_t)0x0000FFFF)  /*!< RTC LSB Mask */
#define PRLH_MSB_MASK    ((uint32_t)0x000F0000)  /*!< RTC Prescaler MSB Mask */

#ifdef __cplusplus
 namespace stm32 {
#endif
	


/**
  * @brief  Enables or disables the specified RTC interrupts.
  * @param  RTC_IT: specifies the RTC interrupts sources to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg RTC_IT_OW: Overflow interrupt
  *     @arg RTC_IT_ALR: Alarm interrupt
  *     @arg RTC_IT_SEC: Second interrupt
  * @param  NewState: new state of the specified RTC interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rtc_it_config(uint16_t rtc_it, bool enable )
{
  if ( enable )
  {
    RTC->CRH |= rtc_it;
  }
  else
  {
    RTC->CRH &= (uint16_t)~rtc_it;
  }
}

/**
  * @brief  Enters the RTC configuration mode.
  * @param  None
  * @retval None
  */
static inline void rtc_enter_config_mode(void)
{
  /* Set the CNF flag to enter in the Configuration Mode */
  RTC->CRL |= RTC_CRL_CNF;
}


/**
  * @brief  Exits from the RTC configuration mode.
  * @param  None
  * @retval None
  */
static inline void rtc_exit_config_mode(void)
{
  /* Reset the CNF flag to exit from the Configuration Mode */
  RTC->CRL &= (uint16_t)~((uint16_t)RTC_CRL_CNF); 
}

/**
  * @brief  Gets the RTC counter value.
  * @param  None
  * @retval RTC counter value.
  */
static inline uint32_t rtc_get_counter(void)
{
	uint16_t tmp;
	uint32_t tmp1, tmp2;
	do {
		tmp = RTC->CNTL;
		tmp1 =  (((uint32_t)RTC->CNTH << 16 ) | tmp) ;
		tmp = RTC->CNTL;
		tmp2 =  (((uint32_t)RTC->CNTH << 16 ) | tmp) ;
	} while( tmp1!=tmp2 );
	return tmp1;
}



/**
  * @brief  Sets the RTC counter value.
  * @param  CounterValue: RTC counter new value.
  * @retval None
  */
static inline void rtc_set_counter(uint32_t CounterValue)
{ 
  rtc_enter_config_mode();
  /* Set RTC COUNTER MSB word */
  RTC->CNTH = CounterValue >> 16;
  /* Set RTC COUNTER LSB word */
  RTC->CNTL = (CounterValue & RTC_LSB_MASK);
  rtc_exit_config_mode();
}


/**
  * @brief  Sets the RTC prescaler value.
  * @param  PrescalerValue: RTC prescaler new value.
  * @retval None
  */
static inline void rtc_set_prescaler(uint32_t PrescalerValue)
{
  
  rtc_enter_config_mode();
  /* Set RTC PRESCALER MSB word */
  RTC->PRLH = (PrescalerValue & PRLH_MSB_MASK) >> 16;
  /* Set RTC PRESCALER LSB word */
  RTC->PRLL = (PrescalerValue & RTC_LSB_MASK);
  rtc_exit_config_mode();
}


/**
  * @brief  Sets the RTC alarm value.
  * @param  AlarmValue: RTC alarm new value.
  * @retval None
  */
static inline void rtc_set_alarm(uint32_t AlarmValue)
{  
  rtc_enter_config_mode();
  /* Set the ALARM MSB word */
  RTC->ALRH = AlarmValue >> 16;
  /* Set the ALARM LSB word */
  RTC->ALRL = (AlarmValue & RTC_LSB_MASK);
  rtc_exit_config_mode();
}

/**
  * @brief  Gets the RTC divider value.
  * @param  None
  * @retval RTC Divider value.
  */
static inline uint32_t rtc_get_divider(void)
{
  uint32_t tmp = 0x00;
  tmp = ((uint32_t)RTC->DIVH & (uint32_t)0x000F) << 16;
  tmp |= RTC->DIVL;
  return tmp;
}

/**
  * @brief  Waits until last write operation on RTC registers has finished.
  * @note   This function must be called before any write to RTC registers.
  * @param  None
  * @retval None
  */
static inline void rtc_wait_for_last_task(void)
{
  /* Loop until RTOFF flag is set */
  while ((RTC->CRL & RTC_FLAG_RTOFF) == 0 )
  {
  }
}

/**
  * @brief  Waits until the RTC registers (RTC_CNT, RTC_ALR and RTC_PRL)
  *   are synchronized with RTC APB clock.
  * @note   This function must be called before any read operation after an APB reset
  *   or an APB clock stop.
  * @param  None
  * @retval None
  */
static inline void rtc_wait_for_synchro(void)
{
  /* Clear RSF flag */
  RTC->CRL &= (uint16_t)~RTC_FLAG_RSF;
  /* Loop until RSF flag is set */
  while ((RTC->CRL & RTC_FLAG_RSF) == 0)
  {
  }
}

/**
  * @brief  Checks whether the specified RTC flag is set or not.
  * @param  RTC_FLAG: specifies the flag to check.
  *   This parameter can be one the following values:
  *     @arg RTC_FLAG_RTOFF: RTC Operation OFF flag
  *     @arg RTC_FLAG_RSF: Registers Synchronized flag
  *     @arg RTC_FLAG_OW: Overflow flag
  *     @arg RTC_FLAG_ALR: Alarm flag
  *     @arg RTC_FLAG_SEC: Second flag
  * @retval The new state of RTC_FLAG (SET or RESET).
  */
static inline bool rtc_get_flag_status(uint16_t RTC_FLAG)
{
  
  return ((RTC->CRL & RTC_FLAG) != 0 );
}


/**
  * @brief  Clears the RTC's pending flags.
  * @param  RTC_FLAG: specifies the flag to clear.
  *   This parameter can be any combination of the following values:
  *     @arg RTC_FLAG_RSF: Registers Synchronized flag. This flag is cleared only after
  *                        an APB reset or an APB Clock stop.
  *     @arg RTC_FLAG_OW: Overflow flag
  *     @arg RTC_FLAG_ALR: Alarm flag
  *     @arg RTC_FLAG_SEC: Second flag
  * @retval None
  */
static inline void rtc_clear_flag(uint16_t RTC_FLAG)
{
  /* Clear the corresponding RTC flag */
  RTC->CRL &= (uint16_t)~RTC_FLAG;
}


/**
  * @brief  Checks whether the specified RTC interrupt has occurred or not.
  * @param  RTC_IT: specifies the RTC interrupts sources to check.
  *   This parameter can be one of the following values:
  *     @arg RTC_IT_OW: Overflow interrupt
  *     @arg RTC_IT_ALR: Alarm interrupt
  *     @arg RTC_IT_SEC: Second interrupt
  * @retval The new state of the RTC_IT (SET or RESET).
  */
static inline bool rtc_get_it_status(uint16_t RTC_IT)
{
  const bool bitstatus = (bool)(RTC->CRL & RTC_IT);
  return (((RTC->CRH & RTC_IT) != 0 ) && (bitstatus != 0 ));
}


/**
  * @brief  Clears the RTC's interrupt pending bits.
  * @param  RTC_IT: specifies the interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg RTC_IT_OW: Overflow interrupt
  *     @arg RTC_IT_ALR: Alarm interrupt
  *     @arg RTC_IT_SEC: Second interrupt
  * @retval None
  */
static inline void rtc_clear_it_pending_bit(uint16_t RTC_IT)
{
  /* Clear the corresponding RTC pending bit */
  RTC->CRL &= (uint16_t)~RTC_IT;
}



#ifdef __cplusplus
 }
#endif

#undef  RTC_LSB_MASK  
#undef  PRLH_MSB_MASK 
