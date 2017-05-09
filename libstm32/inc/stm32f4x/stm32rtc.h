/*
 * stm32rtc.h
 *
 *  Created on: 23-07-2012
 *      Author: lucck
 */

#pragma once

#include <stm32lib.h>

#ifdef __cplusplus
namespace stm32 {
#endif


/* Masks Definition */
#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)
#define RTC_DR_RESERVED_MASK    ((uint32_t)0x00FFFF3F)
#define RTC_INIT_MASK           ((uint32_t)0xFFFFFFFF)
#define RTC_RSF_MASK            ((uint32_t)0xFFFFFF5F)
#define INITMODE_TIMEOUT         ((uint32_t) 0x00010000)
#define SYNCHRO_TIMEOUT          ((uint32_t) 0x00020000)
#if defined(STM32MCU_MAJOR_TYPE_F4)||defined(STM32MCU_MAJOR_TYPE_F37)
#define RECALPF_TIMEOUT          ((uint32_t) 0x00020000)
#define SHPF_TIMEOUT             ((uint32_t) 0x00001000)
#else
#define RECALPF_TIMEOUT          ((uint32_t)0)
#define SHPF_TIMEOUT             ((uint32_t)0)
#endif

#ifndef RTC_FLAG_RECALPF 
#define RTC_FLAG_RECALPF 0
#endif

#ifndef RTC_FLAG_SHPF
#define RTC_FLAG_SHPF 0
#endif

#define RTC_FLAGS_MASK          ((uint32_t)(RTC_FLAG_TSOVF | RTC_FLAG_TSF | RTC_FLAG_WUTF | \
                                            RTC_FLAG_ALRBF | RTC_FLAG_ALRAF | RTC_FLAG_INITF | \
                                            RTC_FLAG_RSF | RTC_FLAG_INITS | RTC_FLAG_WUTWF | \
                                            RTC_FLAG_ALRBWF | RTC_FLAG_ALRAWF | RTC_FLAG_TAMP1F | \
                                            RTC_FLAG_RECALPF | RTC_FLAG_SHPF))


/**
  * @brief  Enters the RTC Initialization mode.
  * @note   The RTC Initialization mode is write protected, use the
  *         RTC_WriteProtectionCmd(DISABLE) before calling this function.
  * @param  None
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: (0) RTC is in Init mode
  *          - ERROR:  (1) RTC is not in Init mode
  */
static inline bool rtc_enter_init_mode(void)
{
  __IO uint32_t initcounter = 0x00;
  uint32_t initstatus = 0x00;

  /* Check if the Initialization mode is set */
  if ( !(RTC->ISR & RTC_ISR_INITF) )
  {
    /* Set the Initialization mode */
    RTC->ISR = (uint32_t)RTC_INIT_MASK;

    /* Wait till RTC is in INIT state and if Time out is reached exit */
    do
    {
      initstatus = RTC->ISR & RTC_ISR_INITF;
      initcounter++;
    } while((initcounter != INITMODE_TIMEOUT) && (initstatus == 0x00));
    if ( RTC->ISR & RTC_ISR_INITF ) {
      return false;
    } else {
      return true;
    }
  }
  else
  {
    return false;
  }
}


/**
  * @brief  Exits the RTC Initialization mode.
  * @note   When the initialization sequence is complete, the calendar restarts
  *         counting after 4 RTCCLK cycles.
  * @note   The RTC Initialization mode is write protected, use the
  *         RTC_WriteProtectionCmd(DISABLE) before calling this function.
  * @param  None
  * @retval None
  */

static inline void rtc_exit_init_mode(void)
{
  /* Exit Initialization mode */
  RTC->ISR &= (uint32_t)~RTC_ISR_INIT;
}


/**
  * @brief  Initializes the RTC registers according to the specified parameters
  *         in RTC_InitStruct.
  * @param  RTC_InitStruct: pointer to a RTC_InitTypeDef structure that contains
  *         the configuration information for the RTC peripheral.
  * @note   The RTC Prescaler register is write protected and can be written in
  *         initialization mode only.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC registers are initialized
  *          - ERROR: RTC registers are not initialized
  */
static inline bool rtc_init(uint32_t hour_format, uint32_t synch_prediv, uint32_t asynch_prediv)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Set Initialization mode */
  if ( rtc_enter_init_mode() )
  {
    return true;
  }
  else
  {
    /* Clear RTC CR FMT Bit */
    RTC->CR &= ((uint32_t)~(RTC_CR_FMT));
    /* Set RTC_CR register */
    RTC->CR |=  ((uint32_t)(hour_format));

    /* Configure the RTC PRER */
    RTC->PRER = (uint32_t)(synch_prediv);
    RTC->PRER |= (uint32_t)(asynch_prediv << 16);

    /* Exit Initialization mode */
    rtc_exit_init_mode();

   return false;
  }
  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}

/**
  * @brief  Enables or disables the RTC registers write protection.
  * @note   All the RTC registers are write protected except for RTC_ISR[13:8],
  *         RTC_TAFCR and RTC_BKPxR.
  * @note   Writing a wrong key reactivates the write protection.
  * @note   The protection mechanism is not affected by system reset.
  * @param  NewState: new state of the write protection.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rtc_write_protection_cmd(bool enable)
{

  if ( enable )
  {
    /* Enable the write protection for RTC registers */
    RTC->WPR = 0xFF;
  }
  else
  {
    /* Disable the write protection for RTC registers */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
  }
}

/**
  * @brief  Waits until the RTC Time and Date registers (RTC_TR and RTC_DR) are
  *         synchronized with RTC APB clock.
  * @note   The RTC Resynchronization mode is write protected, use the
  *         RTC_WriteProtectionCmd(DISABLE) before calling this function.
  * @note   To read the calendar through the shadow registers after Calendar
  *         initialization, calendar update or after wakeup from low power modes
  *         the software must first clear the RSF flag.
  *         The software must then wait until it is set again before reading
  *         the calendar, which means that the calendar registers have been
  *         correctly copied into the RTC_TR and RTC_DR shadow registers.
  * @param  None
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC registers are synchronised
  *          - ERROR: RTC registers are not synchronised
  */
static inline bool rtc_wait_for_synchro(void)
{
  __IO uint32_t synchrocounter = 0;
  uint32_t synchrostatus = 0x00;

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Clear RSF flag */
  RTC->ISR &= (uint32_t)RTC_RSF_MASK;

  /* Wait the registers to be synchronised */
  do
  {
    synchrostatus = RTC->ISR & RTC_ISR_RSF;
    synchrocounter++;
  } while((synchrocounter != SYNCHRO_TIMEOUT) && (synchrostatus == 0x00));

  if ( RTC->ISR & RTC_ISR_RSF ) {
    return false;
  } else {
    return true;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
  return true;
}

/**
  * @brief  Enables or disables the RTC reference clock detection.
  * @param  NewState: new state of the RTC reference clock.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC reference clock detection is enabled
  *          - ERROR: RTC reference clock detection is disabled
  */
static inline bool rtc_ref_clock_cmd( bool enable )
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Set Initialization mode */
  if (rtc_enter_init_mode())
  {
    return true;
  }
  else
  {
    if ( enable )
    {
      /* Enable the RTC reference clock detection */
      RTC->CR |= RTC_CR_REFCKON;
    }
    else
    {
      /* Disable the RTC reference clock detection */
      RTC->CR &= ~RTC_CR_REFCKON;
    }
    /* Exit Initialization mode */
    rtc_exit_init_mode();

    return false;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}

#ifdef RTC_CR_BYPSHAD
/**
  * @brief  Enables or Disables the Bypass Shadow feature.
  * @note   When the Bypass Shadow is enabled the calendar value are taken
  *         directly from the Calendar counter.
  * @param  NewState: new state of the Bypass Shadow feature.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
*/
static inline void rtc_bypass_shadow_cmd(bool enable)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  if ( enable )
  {
    /* Set the BYPSHAD bit */
    RTC->CR |= (uint8_t)RTC_CR_BYPSHAD;
  }
  else
  {
    /* Reset the BYPSHAD bit */
    RTC->CR &= (uint8_t)~RTC_CR_BYPSHAD;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}
#endif

/**
  * @brief  Converts a 2 digit decimal to BCD format.
  * @param  Value: Byte to be converted.
  * @retval Converted byte
  */
static inline uint8_t _private_RTC_ByteToBcd2(uint8_t Value)
{
  uint8_t bcdhigh = 0;

  while (Value >= 10)
  {
    bcdhigh++;
    Value -= 10;
  }

  return  ((uint8_t)(bcdhigh << 4) | Value);
}

/**
  * @brief  Convert from 2 digit BCD to Binary.
  * @param  Value: BCD value to be converted.
  * @retval Converted word
  */
static inline uint8_t _private_RTC_Bcd2ToByte(uint8_t Value)
{
  uint8_t tmp = 0;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
  return (tmp + (Value & (uint8_t)0x0F));
}



/**
  * @brief  Set the RTC current time.
  * @param  RTC_Format: specifies the format of the entered parameters.
  *          This parameter can be  one of the following values:
  *            @arg RTC_Format_BIN:  Binary data format
  *            @arg RTC_Format_BCD:  BCD data format
  * @param  RTC_TimeStruct: pointer to a RTC_TimeTypeDef structure that contains
  *                        the time configuration information for the RTC.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC Time register is configured
  *          - ERROR: RTC Time register is not configured
  */
static inline bool rtc_set_time(uint32_t RTC_Format, uint8_t hours, uint8_t minutes,
		uint8_t seconds, uint8_t H12)
{
  uint32_t tmpreg = 0;


  if (RTC_Format == RTC_Format_BIN)
  {
    if ( (RTC->CR & RTC_CR_FMT) )
    {
    }
    else
    {
      H12 = 0x00;
    }
  }
  else
  {
    if ( (RTC->CR & RTC_CR_FMT) )
    {
      tmpreg = _private_RTC_Bcd2ToByte(hours);
    }
    else
    {
      H12 = 0x00;
    }
  }

  /* Check the input parameters format */
  if (RTC_Format != RTC_Format_BIN)
  {
    tmpreg = (((uint32_t)(hours) << 16) | \
             ((uint32_t)(minutes) << 8) | \
             ((uint32_t)seconds) | \
             ((uint32_t)(H12) << 16));
  }
  else
  {
    tmpreg = (uint32_t)(((uint32_t)_private_RTC_ByteToBcd2(hours) << 16) | \
                   ((uint32_t)_private_RTC_ByteToBcd2(minutes) << 8) | \
                   ((uint32_t)_private_RTC_ByteToBcd2(seconds)) | \
                   (((uint32_t)H12) << 16));
  }

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Set Initialization mode */
  if (rtc_enter_init_mode())
  {
    return true;
  }
  else
  {
    /* Set the RTC_TR register */
    RTC->TR = (uint32_t)(tmpreg & RTC_TR_RESERVED_MASK);

    /* Exit Initialization mode */
    rtc_exit_init_mode();
#ifdef RTC_CR_BYPSHAD
    /* If  RTC_CR_BYPSHAD bit = 0, wait for synchro else this check is not needed */
    if ((RTC->CR & RTC_CR_BYPSHAD) == RESET)
    {
#endif
      if (rtc_wait_for_synchro())
      {
        return true;
      }
      else
      {
        return false;
      }
#ifdef RTC_CR_BYPSHAD
    }
    else
    {
      return true;
    }
#endif
  }
  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}

/**
  * @brief  Get the RTC current Time.
  * @param  RTC_Format: specifies the format of the returned parameters.
  *          This parameter can be  one of the following values:
  *            @arg RTC_Format_BIN:  Binary data format
  *            @arg RTC_Format_BCD:  BCD data format
  * @param  RTC_TimeStruct: pointer to a RTC_TimeTypeDef structure that will
  *                        contain the returned current time configuration.
  * @retval None
  */
static inline void rtc_get_time(uint32_t RTC_Format, uint8_t *hours, uint8_t *minutes,
		uint8_t *seconds, uint8_t *H12)
{
  uint32_t tmpreg = 0;
  /* Get the RTC_TR register */
  tmpreg = (uint32_t)(RTC->TR & RTC_TR_RESERVED_MASK);

  /* Fill the structure fields with the read parameters */
  if(hours) *hours = (uint8_t)((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
  if(minutes) *minutes = (uint8_t)((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
  if(seconds) *seconds = (uint8_t)(tmpreg & (RTC_TR_ST | RTC_TR_SU));
  if(H12) *H12 = (uint8_t)((tmpreg & (RTC_TR_PM)) >> 16);

  /* Check the input parameters format */
  if (RTC_Format == RTC_Format_BIN)
  {
    /* Convert the structure parameters to Binary format */
    if(hours) *hours = (uint8_t)_private_RTC_Bcd2ToByte(*hours);
    if(minutes) *minutes = (uint8_t)_private_RTC_Bcd2ToByte(*minutes);
    if(seconds) *seconds = (uint8_t)_private_RTC_Bcd2ToByte(*seconds);
  }
}

#if defined(STM32MCU_MAJOR_TYPE_F4)|| defined(STM32MCU_MAJOR_TYPE_F37)
/**
  * @brief  Gets the RTC current Calendar Subseconds value.
  * @note   This function freeze the Time and Date registers after reading the
  *         SSR register.
  * @param  None
  * @retval RTC current Calendar Subseconds value.
  */
static inline uint32_t rtc_get_sub_second(void)
{
  uint32_t tmpreg = 0;

  /* Get subseconds values from the correspondent registers*/
  tmpreg = (uint32_t)(RTC->SSR);

  /* Read DR register to unfroze calendar registers */
  (void) (RTC->DR);

  return (tmpreg);
}
#endif

/**
  * @brief  Set the RTC current date.
  * @param  RTC_Format: specifies the format of the entered parameters.
  *          This parameter can be  one of the following values:
  *            @arg RTC_Format_BIN:  Binary data format
  *            @arg RTC_Format_BCD:  BCD data format
  * @param  RTC_DateStruct: pointer to a RTC_DateTypeDef structure that contains
  *                         the date configuration information for the RTC.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC Date register is configured
  *          - ERROR: RTC Date register is not configured
  */
static inline bool rtc_set_date(uint32_t RTC_Format,  uint8_t year,
		uint8_t month, uint8_t date, uint8_t weekday )
{
  uint32_t tmpreg = 0;



  if ((RTC_Format == RTC_Format_BIN) && ((month & 0x10) == 0x10))
  {
    month = (month & (uint32_t)~(0x10)) + 0x0A;
  }

  /* Check the input parameters format */
  if (RTC_Format != RTC_Format_BIN)
  {
    tmpreg = ((((uint32_t)year) << 16) | \
              (((uint32_t)month) << 8) | \
              ((uint32_t)date) | \
              (((uint32_t)weekday) << 13));
  }
  else
  {
    tmpreg = (((uint32_t)_private_RTC_ByteToBcd2(year) << 16) | \
              ((uint32_t)_private_RTC_ByteToBcd2(month) << 8) | \
              ((uint32_t)_private_RTC_ByteToBcd2(date)) | \
              ((uint32_t)weekday << 13));
  }

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Set Initialization mode */
  if (rtc_enter_init_mode())
  {
    return true;
  }
  else
  {
    /* Set the RTC_DR register */
    RTC->DR = (uint32_t)(tmpreg & RTC_DR_RESERVED_MASK);

    /* Exit Initialization mode */
    rtc_exit_init_mode();
#ifdef RTC_CR_BYPSHAD
    /* If  RTC_CR_BYPSHAD bit = 0, wait for synchro else this check is not needed */
    if ((RTC->CR & RTC_CR_BYPSHAD) == RESET)
    {
#endif
      if (rtc_wait_for_synchro() )
      {
        return true;
      }
      else
      {
        return false;
      }
#ifdef RTC_CR_BYPSHAD
    }
    else
    {
      return true;
    }
#endif
  }
  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
  return true;
}

/**
  * @brief  Get the RTC current date.
  * @param  RTC_Format: specifies the format of the returned parameters.
  *          This parameter can be one of the following values:
  *            @arg RTC_Format_BIN: Binary data format
  *            @arg RTC_Format_BCD: BCD data format
  * @param RTC_DateStruct: pointer to a RTC_DateTypeDef structure that will
  *                        contain the returned current date configuration.
  * @retval None
  */
static inline void rtc_get_date(uint32_t RTC_Format, uint8_t *year,
		uint8_t *month, uint8_t *date, uint8_t *weekday)
{
  uint32_t tmpreg = 0;


  /* Get the RTC_TR register */
  tmpreg = (uint32_t)(RTC->DR & RTC_DR_RESERVED_MASK);

  /* Fill the structure fields with the read parameters */
  if(year)  	*year = (uint8_t)((tmpreg & (RTC_DR_YT | RTC_DR_YU)) >> 16);
  if(month) 	*month = (uint8_t)((tmpreg & (RTC_DR_MT | RTC_DR_MU)) >> 8);
  if(date)  	*date= (uint8_t)(tmpreg & (RTC_DR_DT | RTC_DR_DU));
  if(weekday) 	*weekday = (uint8_t)((tmpreg & (RTC_DR_WDU)) >> 13);

  /* Check the input parameters format */
  if (RTC_Format == RTC_Format_BIN)
  {
    /* Convert the structure parameters to Binary format */
	if(year)  	*year  = (uint8_t)_private_RTC_Bcd2ToByte( *year );
    if(month)   *month = (uint8_t)_private_RTC_Bcd2ToByte( *month );
    if(date)    *date = (uint8_t)_private_RTC_Bcd2ToByte( *date );
  }
}

/**
  * @brief  Set the specified RTC Alarm.
  * @note   The Alarm register can only be written when the corresponding Alarm
  *         is disabled (Use the RTC_AlarmCmd(DISABLE)).
  * @param  RTC_Format: specifies the format of the returned parameters.
  *          This parameter can be one of the following values:
  *            @arg RTC_Format_BIN: Binary data format
  *            @arg RTC_Format_BCD: BCD data format
  * @param  RTC_Alarm: specifies the alarm to be configured.
  *          This parameter can be one of the following values:
  *            @arg RTC_Alarm_A: to select Alarm A
  *            @arg RTC_Alarm_B: to select Alarm B
  * @param  RTC_AlarmStruct: pointer to a RTC_AlarmTypeDef structure that
  *                          contains the alarm configuration parameters.
  * @retval None
  */
static inline void rtc_set_alarm(uint32_t RTC_Format, uint32_t RTC_Alarm,
		uint32_t rtc_alarm_mask, uint32_t rtc_alarm_weekday_sel, uint32_t rtc_alarm_date_weekday,
		uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t h12)
{
  uint32_t tmpreg = 0;

  if (RTC_Format == RTC_Format_BIN)
  {
    if ( (RTC->CR & RTC_CR_FMT) )
    {
    }
    else
    {
      h12 = 0x00;
    }
  }
  else
  {
    if ( (RTC->CR & RTC_CR_FMT) )
    {
      tmpreg = _private_RTC_Bcd2ToByte(hours);
    }
    else
    {
      h12 = 0x00;
    }


    if(rtc_alarm_weekday_sel == RTC_AlarmDateWeekDaySel_Date)
    {
      tmpreg = _private_RTC_Bcd2ToByte(rtc_alarm_date_weekday);
    }
    else
    {
      tmpreg = _private_RTC_Bcd2ToByte(rtc_alarm_date_weekday);
    }
  }

  /* Check the input parameters format */
  if (RTC_Format != RTC_Format_BIN)
  {
    tmpreg = (((uint32_t)(hours) << 16) | \
              ((uint32_t)(minutes) << 8) | \
              ((uint32_t)seconds) | \
              ((uint32_t)(h12) << 16) | \
              ((uint32_t)(rtc_alarm_date_weekday) << 24) | \
              ((uint32_t)rtc_alarm_weekday_sel) | \
              ((uint32_t)rtc_alarm_mask));
  }
  else
  {
    tmpreg = (((uint32_t)_private_RTC_ByteToBcd2(hours) << 16) | \
              ((uint32_t)_private_RTC_ByteToBcd2(minutes) << 8) | \
              ((uint32_t)_private_RTC_ByteToBcd2(seconds)) | \
              ((uint32_t)(h12) << 16) | \
              ((uint32_t)_private_RTC_ByteToBcd2(rtc_alarm_date_weekday) << 24) | \
              ((uint32_t)rtc_alarm_weekday_sel) | \
              ((uint32_t)rtc_alarm_mask));
  }

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Configure the Alarm register */
  if (RTC_Alarm == RTC_Alarm_A)
  {
    RTC->ALRMAR = (uint32_t)tmpreg;
  }
  else
  {
    RTC->ALRMBR = (uint32_t)tmpreg;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

/**
  * @brief  Get the RTC Alarm value and masks.
  * @param  RTC_Format: specifies the format of the output parameters.
  *          This parameter can be one of the following values:
  *            @arg RTC_Format_BIN: Binary data format
  *            @arg RTC_Format_BCD: BCD data format
  * @param  RTC_Alarm: specifies the alarm to be read.
  *          This parameter can be one of the following values:
  *            @arg RTC_Alarm_A: to select Alarm A
  *            @arg RTC_Alarm_B: to select Alarm B
  * @param  RTC_AlarmStruct: pointer to a RTC_AlarmTypeDef structure that will
  *                          contains the output alarm configuration values.
  * @retval None
  */
static inline void rtc_get_alarm(uint32_t RTC_Format, uint32_t RTC_Alarm,
		uint32_t *rtc_alarm_mask, uint32_t *rtc_alarm_weekday_sel, uint32_t *rtc_alarm_date_weekday,
		uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *h12)
{
  uint32_t tmpreg = 0;
  /* Get the RTC_ALRMxR register */
  if (RTC_Alarm == RTC_Alarm_A)
  {
    tmpreg = (uint32_t)(RTC->ALRMAR);
  }
  else
  {
    tmpreg = (uint32_t)(RTC->ALRMBR);
  }

  /* Fill the structure with the read parameters */
  if(hours)   *hours   = (uint32_t)((tmpreg & (RTC_ALRMAR_HT | RTC_ALRMAR_HU)) >> 16);
  if(minutes) *minutes = (uint32_t)((tmpreg & (RTC_ALRMAR_MNT | RTC_ALRMAR_MNU)) >> 8);
  if(seconds) *seconds = (uint32_t)(tmpreg & (RTC_ALRMAR_ST | RTC_ALRMAR_SU));
  if(h12)     *h12 = 	 (uint32_t)((tmpreg & RTC_ALRMAR_PM) >> 16);
  if(rtc_alarm_date_weekday) *rtc_alarm_date_weekday = (uint32_t)((tmpreg & (RTC_ALRMAR_DT | RTC_ALRMAR_DU)) >> 24);
  if(rtc_alarm_weekday_sel)  *rtc_alarm_weekday_sel= (uint32_t)(tmpreg & RTC_ALRMAR_WDSEL);
  if(rtc_alarm_mask) *rtc_alarm_mask = (uint32_t)(tmpreg & RTC_AlarmMask_All);

  if (RTC_Format == RTC_Format_BIN)
  {
    if(hours)  *hours = _private_RTC_Bcd2ToByte(*hours);
    if(minutes) *minutes = _private_RTC_Bcd2ToByte(*minutes);
    if(seconds) *seconds = _private_RTC_Bcd2ToByte(*seconds);
    if(rtc_alarm_date_weekday) *rtc_alarm_date_weekday = _private_RTC_Bcd2ToByte(*rtc_alarm_date_weekday);
  }
}

/**
  * @brief  Enables or disables the specified RTC Alarm.
  * @param  RTC_Alarm: specifies the alarm to be configured.
  *          This parameter can be any combination of the following values:
  *            @arg RTC_Alarm_A: to select Alarm A
  *            @arg RTC_Alarm_B: to select Alarm B
  * @param  NewState: new state of the specified alarm.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC Alarm is enabled/disabled
  *          - ERROR: RTC Alarm is not enabled/disabled
  */
static inline bool rtc_alarm_cmd(uint32_t RTC_Alarm, bool enable)
{
  __IO uint32_t alarmcounter = 0x00;
  uint32_t alarmstatus = 0x00;

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Configure the Alarm state */
  if (enable)
  {
    RTC->CR |= (uint32_t)RTC_Alarm;
    return false;
  }
  else
  {
    /* Disable the Alarm in RTC_CR register */
    RTC->CR &= (uint32_t)~RTC_Alarm;

    /* Wait till RTC ALRxWF flag is set and if Time out is reached exit */
    do
    {
      alarmstatus = RTC->ISR & (RTC_Alarm >> 8);
      alarmcounter++;
    } while((alarmcounter != INITMODE_TIMEOUT) && (alarmstatus == 0x00));

    if (!(RTC->ISR & (RTC_Alarm >> 8)) )
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}

/**
  * @brief  Configure the RTC AlarmA/B Subseconds value and mask.*
  * @note   This function is performed only when the Alarm is disabled.
  * @param  RTC_Alarm: specifies the alarm to be configured.
  *   This parameter can be one of the following values:
  *     @arg RTC_Alarm_A: to select Alarm A
  *     @arg RTC_Alarm_B: to select Alarm B
  * @param  RTC_AlarmSubSecondValue: specifies the Subseconds value.
  *   This parameter can be a value from 0 to 0x00007FFF.
  * @param  RTC_AlarmSubSecondMask:  specifies the Subseconds Mask.
  *   This parameter can be any combination of the following values:
  *     @arg RTC_AlarmSubSecondMask_All    : All Alarm SS fields are masked.
  *                                          There is no comparison on sub seconds for Alarm.
  *     @arg RTC_AlarmSubSecondMask_SS14_1 : SS[14:1] are don't care in Alarm comparison.
  *                                          Only SS[0] is compared
  *     @arg RTC_AlarmSubSecondMask_SS14_2 : SS[14:2] are don't care in Alarm comparison.
  *                                          Only SS[1:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_3 : SS[14:3] are don't care in Alarm comparison.
  *                                          Only SS[2:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_4 : SS[14:4] are don't care in Alarm comparison.
  *                                          Only SS[3:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_5 : SS[14:5] are don't care in Alarm comparison.
  *                                          Only SS[4:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_6 : SS[14:6] are don't care in Alarm comparison.
  *                                          Only SS[5:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_7 : SS[14:7] are don't care in Alarm comparison.
  *                                          Only SS[6:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_8 : SS[14:8] are don't care in Alarm comparison.
  *                                          Only SS[7:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_9 : SS[14:9] are don't care in Alarm comparison.
  *                                          Only SS[8:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_10: SS[14:10] are don't care in Alarm comparison.
  *                                          Only SS[9:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_11: SS[14:11] are don't care in Alarm comparison.
  *                                          Only SS[10:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_12: SS[14:12] are don't care in Alarm comparison.
  *                                          Only SS[11:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14_13: SS[14:13] are don't care in Alarm comparison.
  *                                          Only SS[12:0] are compared
  *     @arg RTC_AlarmSubSecondMask_SS14   : SS[14] is don't care in Alarm comparison.
  *                                          Only SS[13:0] are compared
  *     @arg RTC_AlarmSubSecondMask_None   : SS[14:0] are compared and must match
  *                                          to activate alarm
  * @retval None
  */
#ifdef RTC_AlarmSubSecondMask_All
static inline void rtc_alarm_sub_second_config(uint32_t RTC_Alarm, uint32_t RTC_AlarmSubSecondValue, uint32_t RTC_AlarmSubSecondMask)
{
  uint32_t tmpreg = 0;


  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Configure the Alarm A or Alarm B SubSecond registers */
  tmpreg = (uint32_t) (uint32_t)(RTC_AlarmSubSecondValue) | (uint32_t)(RTC_AlarmSubSecondMask);

  if (RTC_Alarm == RTC_Alarm_A)
  {
    /* Configure the AlarmA SubSecond register */
    RTC->ALRMASSR = tmpreg;
  }
  else
  {
    /* Configure the Alarm B SubSecond register */
    RTC->ALRMBSSR = tmpreg;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

}

#endif

/**
  * @brief  Gets the RTC Alarm Subseconds value.
  * @param  RTC_Alarm: specifies the alarm to be read.
  *   This parameter can be one of the following values:
  *     @arg RTC_Alarm_A: to select Alarm A
  *     @arg RTC_Alarm_B: to select Alarm B
  * @param  None
  * @retval RTC Alarm Subseconds value.
  */
#ifdef RTC_ALRMASSR_SS
static inline uint32_t rtc_get_alarm_sub_second(uint32_t RTC_Alarm)
{
  uint32_t tmpreg = 0;

  /* Get the RTC_ALRMxR register */
  if (RTC_Alarm == RTC_Alarm_A)
  {
    tmpreg = (uint32_t)((RTC->ALRMASSR) & RTC_ALRMASSR_SS);
  }
  else
  {
    tmpreg = (uint32_t)((RTC->ALRMBSSR) & RTC_ALRMBSSR_SS);
  }

  return (tmpreg);
}
#endif

/**
  * @brief  Configures the RTC Wakeup clock source.
  * @note   The WakeUp Clock source can only be changed when the RTC WakeUp
  *         is disabled (Use the RTC_WakeUpCmd(DISABLE)).
  * @param  RTC_WakeUpClock: Wakeup Clock source.
  *          This parameter can be one of the following values:
  *            @arg RTC_WakeUpClock_RTCCLK_Div16: RTC Wakeup Counter Clock = RTCCLK/16
  *            @arg RTC_WakeUpClock_RTCCLK_Div8: RTC Wakeup Counter Clock = RTCCLK/8
  *            @arg RTC_WakeUpClock_RTCCLK_Div4: RTC Wakeup Counter Clock = RTCCLK/4
  *            @arg RTC_WakeUpClock_RTCCLK_Div2: RTC Wakeup Counter Clock = RTCCLK/2
  *            @arg RTC_WakeUpClock_CK_SPRE_16bits: RTC Wakeup Counter Clock = CK_SPRE
  *            @arg RTC_WakeUpClock_CK_SPRE_17bits: RTC Wakeup Counter Clock = CK_SPRE
  * @retval None
  */
static inline void rtc_wake_up_clock_config(uint32_t RTC_WakeUpClock)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Clear the Wakeup Timer clock source bits in CR register */
  RTC->CR &= (uint32_t)~RTC_CR_WUCKSEL;

  /* Configure the clock source */
  RTC->CR |= (uint32_t)RTC_WakeUpClock;

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

/**
  * @brief  Configures the RTC Wakeup counter.
  * @note   The RTC WakeUp counter can only be written when the RTC WakeUp
  *         is disabled (Use the RTC_WakeUpCmd(DISABLE)).
  * @param  RTC_WakeUpCounter: specifies the WakeUp counter.
  *          This parameter can be a value from 0x0000 to 0xFFFF.
  * @retval None
  */
static inline void rtc_set_wake_up_counter(uint32_t RTC_WakeUpCounter)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Configure the Wakeup Timer counter */
  RTC->WUTR = (uint32_t)RTC_WakeUpCounter;

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

/**
  * @brief  Returns the RTC WakeUp timer counter value.
  * @param  None
  * @retval The RTC WakeUp Counter value.
  */
static inline uint32_t rtc_get_wake_up_counter(void)
{
  /* Get the counter value */
  return ((uint32_t)(RTC->WUTR & RTC_WUTR_WUT));
}

/**
  * @brief  Enables or Disables the RTC WakeUp timer.
  * @param  NewState: new state of the WakeUp timer.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline bool rtc_wake_up_cmd(bool enable)
{
  __IO uint32_t wutcounter = 0x00;
  uint32_t wutwfstatus = 0x00;


  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  if ( enable )
  {
    /* Enable the Wakeup Timer */
    RTC->CR |= (uint32_t)RTC_CR_WUTE;
    return false;
  }
  else
  {
    /* Disable the Wakeup Timer */
    RTC->CR &= (uint32_t)~RTC_CR_WUTE;
    /* Wait till RTC WUTWF flag is set and if Time out is reached exit */
    do
    {
      wutwfstatus = RTC->ISR & RTC_ISR_WUTWF;
      wutcounter++;
    } while((wutcounter != INITMODE_TIMEOUT) && (wutwfstatus == 0x00));

    if (!(RTC->ISR & RTC_ISR_WUTWF) )
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}

/**
  * @brief  Adds or substract one hour from the current time.
  * @param  RTC_DayLightSaveOperation: the value of hour adjustment.
  *          This parameter can be one of the following values:
  *            @arg RTC_DayLightSaving_SUB1H: Substract one hour (winter time)
  *            @arg RTC_DayLightSaving_ADD1H: Add one hour (summer time)
  * @param  RTC_StoreOperation: Specifies the value to be written in the BCK bit
  *                            in CR register to store the operation.
  *          This parameter can be one of the following values:
  *            @arg RTC_StoreOperation_Reset: BCK Bit Reset
  *            @arg RTC_StoreOperation_Set: BCK Bit Set
  * @retval None
  */
static inline void rtc_day_light_saving_config(uint32_t RTC_DayLightSaving, uint32_t RTC_StoreOperation)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Clear the bits to be configured */
  RTC->CR &= (uint32_t)~(RTC_CR_BCK);

  /* Configure the RTC_CR register */
  RTC->CR |= (uint32_t)(RTC_DayLightSaving | RTC_StoreOperation);

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

/**
  * @brief  Returns the RTC Day Light Saving stored operation.
  * @param  None
  * @retval RTC Day Light Saving stored operation.
  *          - RTC_StoreOperation_Reset
  *          - RTC_StoreOperation_Set
  */
static inline uint32_t rtc_get_store_operation(void)
{
  return (RTC->CR & RTC_CR_BCK);
}

/**
  * @brief  Configures the RTC output source (AFO_ALARM).
  * @param  RTC_Output: Specifies which signal will be routed to the RTC output.
  *          This parameter can be one of the following values:
  *            @arg RTC_Output_Disable: No output selected
  *            @arg RTC_Output_AlarmA: signal of AlarmA mapped to output
  *            @arg RTC_Output_AlarmB: signal of AlarmB mapped to output
  *            @arg RTC_Output_WakeUp: signal of WakeUp mapped to output
  * @param  RTC_OutputPolarity: Specifies the polarity of the output signal.
  *          This parameter can be one of the following:
  *            @arg RTC_OutputPolarity_High: The output pin is high when the
  *                                 ALRAF/ALRBF/WUTF is high (depending on OSEL)
  *            @arg RTC_OutputPolarity_Low: The output pin is low when the
  *                                 ALRAF/ALRBF/WUTF is high (depending on OSEL)
  * @retval None
  */
static inline void rtc_output_config(uint32_t RTC_Output, uint32_t RTC_OutputPolarity)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Clear the bits to be configured */
  RTC->CR &= (uint32_t)~(RTC_CR_OSEL | RTC_CR_POL);

  /* Configure the output selection and polarity */
  RTC->CR |= (uint32_t)(RTC_Output | RTC_OutputPolarity);

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

/**
  * @brief  Configures the Coarse calibration parameters.
  * @param  RTC_CalibSign: specifies the sign of the coarse calibration value.
  *          This parameter can be  one of the following values:
  *            @arg RTC_CalibSign_Positive: The value sign is positive
  *            @arg RTC_CalibSign_Negative: The value sign is negative
  * @param  Value: value of coarse calibration expressed in ppm (coded on 5 bits).
  *
  * @note   This Calibration value should be between 0 and 63 when using negative
  *         sign with a 2-ppm step.
  *
  * @note   This Calibration value should be between 0 and 126 when using positive
  *         sign with a 4-ppm step.
  *
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC Coarse calibration are initialized
  *          - ERROR: RTC Coarse calibration are not initialized
  */
#ifdef RTC_CalibSign_Positive
static inline bool rtc_coarse_calib_config(uint32_t RTC_CalibSign, uint32_t Value)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Set Initialization mode */
  if (rtc_enter_init_mode())
  {
    return true;
  }
  else
  {
    /* Set the coarse calibration value */
    RTC->CALIBR = (uint32_t)(RTC_CalibSign | Value);
    /* Exit Initialization mode */
    rtc_exit_init_mode();

    return false;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}
#endif

/**
  * @brief  Enables or disables the Coarse calibration process.
  * @param  NewState: new state of the Coarse calibration.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC Coarse calibration are enabled/disabled
  *          - ERROR: RTC Coarse calibration are not enabled/disabled
  */
#ifdef RTC_CR_DCE
static inline bool rtc_coarse_calib_cmd(bool enable)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Set Initialization mode */
  if (rtc_enter_init_mode())
  {
    return true;
  }
  else
  {
    if (enable)
    {
      /* Enable the Coarse Calibration */
      RTC->CR |= (uint32_t)RTC_CR_DCE;
    }
    else
    {
      /* Disable the Coarse Calibration */
      RTC->CR &= (uint32_t)~RTC_CR_DCE;
    }
    /* Exit Initialization mode */
    rtc_exit_init_mode();

    return false;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}
#endif

/**
  * @brief  Enables or disables the RTC clock to be output through the relative pin.
  * @param  NewState: new state of the digital calibration Output.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rtc_calib_output_cmd(bool enable)
{
  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  if (enable)
  {
    /* Enable the RTC clock output */
    RTC->CR |= (uint32_t)RTC_CR_COE;
  }
  else
  {
    /* Disable the RTC clock output */
    RTC->CR &= (uint32_t)~RTC_CR_COE;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

#ifdef RTC_CR_COSEL
/**
  * @brief  Configure the Calibration Pinout (RTC_CALIB) Selection (1Hz or 512Hz).
  * @param  RTC_CalibOutput : Select the Calibration output Selection .
  *   This parameter can be one of the following values:
  *     @arg RTC_CalibOutput_512Hz: A signal has a regular waveform at 512Hz.
  *     @arg RTC_CalibOutput_1Hz  : A signal has a regular waveform at 1Hz.
  * @retval None
*/
static inline void rtc_calib_output_config(uint32_t RTC_CalibOutput)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /*clear flags before config*/
  RTC->CR &= (uint32_t)~(RTC_CR_COSEL);

  /* Configure the RTC_CR register */
  RTC->CR |= (uint32_t)RTC_CalibOutput;

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

#endif

#ifdef RTC_ISR_RECALPF
/**
  * @brief  Configures the Smooth Calibration Settings.
  * @param  RTC_SmoothCalibPeriod : Select the Smooth Calibration Period.
  *   This parameter can be can be one of the following values:
  *     @arg RTC_SmoothCalibPeriod_32sec : The smooth calibration periode is 32s.
  *     @arg RTC_SmoothCalibPeriod_16sec : The smooth calibration periode is 16s.
  *     @arg RTC_SmoothCalibPeriod_8sec  : The smooth calibartion periode is 8s.
  * @param  RTC_SmoothCalibPlusPulses : Select to Set or reset the CALP bit.
  *   This parameter can be one of the following values:
  *     @arg RTC_SmoothCalibPlusPulses_Set  : Add one RTCCLK puls every 2**11 pulses.
  *     @arg RTC_SmoothCalibPlusPulses_Reset: No RTCCLK pulses are added.
  * @param  RTC_SmouthCalibMinusPulsesValue: Select the value of CALM[8:0] bits.
  *   This parameter can be one any value from 0 to 0x000001FF.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC Calib registers are configured
  *          - ERROR: RTC Calib registers are not configured
*/
static inline bool rtc_smooth_calib_config(uint32_t RTC_SmoothCalibPeriod,
                                  uint32_t RTC_SmoothCalibPlusPulses,
                                  uint32_t RTC_SmouthCalibMinusPulsesValue)
{
  uint32_t recalpfcount = 0;

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* check if a calibration is pending*/
  if ((RTC->ISR & RTC_ISR_RECALPF) != RESET)
  {
    /* wait until the Calibration is completed*/
    while (((RTC->ISR & RTC_ISR_RECALPF) != RESET) && (recalpfcount != RECALPF_TIMEOUT))
    {
      recalpfcount++;
    }
  }

  /* check if the calibration pending is completed or if there is no calibration operation at all*/
  if ((RTC->ISR & RTC_ISR_RECALPF) == RESET)
  {
    /* Configure the Smooth calibration settings */
    RTC->CALR = (uint32_t)((uint32_t)RTC_SmoothCalibPeriod | (uint32_t)RTC_SmoothCalibPlusPulses | (uint32_t)RTC_SmouthCalibMinusPulsesValue);

    return false;
  }
  else
  {
    return true;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}
#endif

/**
  * @brief  Enables or Disables the RTC TimeStamp functionality with the
  *         specified time stamp pin stimulating edge.
  * @param  RTC_TimeStampEdge: Specifies the pin edge on which the TimeStamp is
  *         activated.
  *          This parameter can be one of the following:
  *            @arg RTC_TimeStampEdge_Rising: the Time stamp event occurs on the rising
  *                                    edge of the related pin.
  *            @arg RTC_TimeStampEdge_Falling: the Time stamp event occurs on the
  *                                     falling edge of the related pin.
  * @param  NewState: new state of the TimeStamp.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rtc_time_stamp_cmd(uint32_t RTC_TimeStampEdge, bool enable)
{
  uint32_t tmpreg = 0;

  /* Get the RTC_CR register and clear the bits to be configured */
  tmpreg = (uint32_t)(RTC->CR & (uint32_t)~(RTC_CR_TSEDGE | RTC_CR_TSE));

  /* Get the new configuration */
  if (enable)
  {
    tmpreg |= (uint32_t)(RTC_TimeStampEdge | RTC_CR_TSE);
  }
  else
  {
    tmpreg |= (uint32_t)(RTC_TimeStampEdge);
  }

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Configure the Time Stamp TSEDGE and Enable bits */
  RTC->CR = (uint32_t)tmpreg;

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

/**
  * @brief  Get the RTC TimeStamp value and masks.
  * @param  RTC_Format: specifies the format of the output parameters.
  *          This parameter can be one of the following values:
  *            @arg RTC_Format_BIN: Binary data format
  *            @arg RTC_Format_BCD: BCD data format
  * @param RTC_StampTimeStruct: pointer to a RTC_TimeTypeDef structure that will
  *                             contains the TimeStamp time values.
  * @param RTC_StampDateStruct: pointer to a RTC_DateTypeDef structure that will
  *                             contains the TimeStamp date values.
  * @retval None
  */
static inline void rtc_get_time_stamp(uint32_t RTC_Format, uint8_t *hours,
		uint8_t *minutes, uint8_t *seconds, uint8_t *h12, uint8_t *year,
		uint8_t *month, uint8_t *date, uint8_t *weekday)
{
  uint32_t tmptime = 0, tmpdate = 0;

  /* Get the TimeStamp time and date registers values */
  tmptime = (uint32_t)(RTC->TSTR & RTC_TR_RESERVED_MASK);
  tmpdate = (uint32_t)(RTC->TSDR & RTC_DR_RESERVED_MASK);

  /* Fill the Time structure fields with the read parameters */
  if(hours) *hours = (uint8_t)((tmptime & (RTC_TR_HT | RTC_TR_HU)) >> 16);
  if(minutes) *minutes = (uint8_t)((tmptime & (RTC_TR_MNT | RTC_TR_MNU)) >> 8);
  if(seconds) *seconds = (uint8_t)(tmptime & (RTC_TR_ST | RTC_TR_SU));
  if(h12) *h12 = (uint8_t)((tmptime & (RTC_TR_PM)) >> 16);

  /* Fill the Date structure fields with the read parameters */
  if(year)  *year= 0;
  if(month) *month = (uint8_t)((tmpdate & (RTC_DR_MT | RTC_DR_MU)) >> 8);
  if(date)  *date = (uint8_t)(tmpdate & (RTC_DR_DT | RTC_DR_DU));
  if(weekday) *weekday = (uint8_t)((tmpdate & (RTC_DR_WDU)) >> 13);

  /* Check the input parameters format */
  if (RTC_Format == RTC_Format_BIN)
  {
    /* Convert the Time structure parameters to Binary format */
    if(hours) *hours = (uint8_t)_private_RTC_Bcd2ToByte(*hours);
    if(minutes) *minutes = (uint8_t)_private_RTC_Bcd2ToByte(*minutes);
    if(seconds) *seconds = (uint8_t)_private_RTC_Bcd2ToByte(*seconds);

    /* Convert the Date structure parameters to Binary format */
    if(month) *month = (uint8_t)_private_RTC_Bcd2ToByte(*month);
    if(date) *date = (uint8_t)_private_RTC_Bcd2ToByte(*date);
    if(weekday) *weekday = (uint8_t)_private_RTC_Bcd2ToByte(*weekday);
  }
}

#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F37)

/**
  * @brief  Get the RTC timestamp Subseconds value.
  * @param  None
  * @retval RTC current timestamp Subseconds value.
  */
static inline uint32_t rtc_get_time_stamp_sub_second(void)
{
  /* Get timestamp subseconds values from the correspondent registers */
  return (uint32_t)(RTC->TSSSR);
}

#endif

/**
  * @brief  Configures the select Tamper pin edge.
  * @param  RTC_Tamper: Selected tamper pin.
  *          This parameter can be RTC_Tamper_1.
  * @param  RTC_TamperTrigger: Specifies the trigger on the tamper pin that
  *         stimulates tamper event.
  *   This parameter can be one of the following values:
  *     @arg RTC_TamperTrigger_RisingEdge: Rising Edge of the tamper pin causes tamper event.
  *     @arg RTC_TamperTrigger_FallingEdge: Falling Edge of the tamper pin causes tamper event.
  *     @arg RTC_TamperTrigger_LowLevel: Low Level of the tamper pin causes tamper event.
  *     @arg RTC_TamperTrigger_HighLevel: High Level of the tamper pin causes tamper event.
  * @retval None
  */
static inline void rtc_tamper_trigger_config(uint32_t RTC_Tamper, uint32_t RTC_TamperTrigger)
{

  if (RTC_TamperTrigger == RTC_TamperTrigger_RisingEdge)
  {
    /* Configure the RTC_TAFCR register */
    RTC->TAFCR &= (uint32_t)((uint32_t)~(RTC_Tamper << 1));
  }
  else
  {
    /* Configure the RTC_TAFCR register */
    RTC->TAFCR |= (uint32_t)(RTC_Tamper << 1);
  }
}

/**
  * @brief  Enables or Disables the Tamper detection.
  * @param  RTC_Tamper: Selected tamper pin.
  *          This parameter can be RTC_Tamper_1.
  * @param  NewState: new state of the tamper pin.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rtc_tamper_cmd(uint32_t RTC_Tamper, bool enable)
{

  if (enable)
  {
    /* Enable the selected Tamper pin */
    RTC->TAFCR |= (uint32_t)RTC_Tamper;
  }
  else
  {
    /* Disable the selected Tamper pin */
    RTC->TAFCR &= (uint32_t)~RTC_Tamper;
  }
}

#ifdef RTC_TAFCR_TAMPFLT
/**
  * @brief  Configures the Tampers Filter.
  * @param  RTC_TamperFilter: Specifies the tampers filter.
  *   This parameter can be one of the following values:
  *     @arg RTC_TamperFilter_Disable: Tamper filter is disabled.
  *     @arg RTC_TamperFilter_2Sample: Tamper is activated after 2 consecutive
  *                                    samples at the active level
  *     @arg RTC_TamperFilter_4Sample: Tamper is activated after 4 consecutive
  *                                    samples at the active level
  *     @arg RTC_TamperFilter_8Sample: Tamper is activated after 8 consecutive
  *                                    samples at the active level
  * @retval None
  */
static inline void rtc_tamper_filter_config(uint32_t RTC_TamperFilter)
{

  /* Clear TAMPFLT[1:0] bits in the RTC_TAFCR register */
  RTC->TAFCR &= (uint32_t)~(RTC_TAFCR_TAMPFLT);

  /* Configure the RTC_TAFCR register */
  RTC->TAFCR |= (uint32_t)RTC_TamperFilter;
}

/**
  * @brief  Configures the Tampers Sampling Frequency.
  * @param  RTC_TamperSamplingFreq: Specifies the tampers Sampling Frequency.
  *   This parameter can be one of the following values:
  *     @arg RTC_TamperSamplingFreq_RTCCLK_Div32768: Each of the tamper inputs are sampled
  *                                           with a frequency =  RTCCLK / 32768
  *     @arg RTC_TamperSamplingFreq_RTCCLK_Div16384: Each of the tamper inputs are sampled
  *                                           with a frequency =  RTCCLK / 16384
  *     @arg RTC_TamperSamplingFreq_RTCCLK_Div8192: Each of the tamper inputs are sampled
  *                                           with a frequency =  RTCCLK / 8192
  *     @arg RTC_TamperSamplingFreq_RTCCLK_Div4096: Each of the tamper inputs are sampled
  *                                           with a frequency =  RTCCLK / 4096
  *     @arg RTC_TamperSamplingFreq_RTCCLK_Div2048: Each of the tamper inputs are sampled
  *                                           with a frequency =  RTCCLK / 2048
  *     @arg RTC_TamperSamplingFreq_RTCCLK_Div1024: Each of the tamper inputs are sampled
  *                                           with a frequency =  RTCCLK / 1024
  *     @arg RTC_TamperSamplingFreq_RTCCLK_Div512: Each of the tamper inputs are sampled
  *                                           with a frequency =  RTCCLK / 512
  *     @arg RTC_TamperSamplingFreq_RTCCLK_Div256: Each of the tamper inputs are sampled
  *                                           with a frequency =  RTCCLK / 256
  * @retval None
  */
static inline void rtc_tamper_sampling_freq_config(uint32_t RTC_TamperSamplingFreq)
{

  /* Clear TAMPFREQ[2:0] bits in the RTC_TAFCR register */
  RTC->TAFCR &= (uint32_t)~(RTC_TAFCR_TAMPFREQ);

  /* Configure the RTC_TAFCR register */
  RTC->TAFCR |= (uint32_t)RTC_TamperSamplingFreq;
}

/**
  * @brief  Configures the Tampers Pins input Precharge Duration.
  * @param  RTC_TamperPrechargeDuration: Specifies the Tampers Pins input
  *         Precharge Duration.
  *   This parameter can be one of the following values:
  *     @arg RTC_TamperPrechargeDuration_1RTCCLK: Tamper pins are pre-charged before sampling during 1 RTCCLK cycle
  *     @arg RTC_TamperPrechargeDuration_2RTCCLK: Tamper pins are pre-charged before sampling during 2 RTCCLK cycle
  *     @arg RTC_TamperPrechargeDuration_4RTCCLK: Tamper pins are pre-charged before sampling during 4 RTCCLK cycle
  *     @arg RTC_TamperPrechargeDuration_8RTCCLK: Tamper pins are pre-charged before sampling during 8 RTCCLK cycle
  * @retval None
  */
/**
  * @brief  Configures the Tampers Pins input Precharge Duration.
  * @param  RTC_TamperPrechargeDuration: Specifies the Tampers Pins input
  *         Precharge Duration.
  *   This parameter can be one of the following values:
  *     @arg RTC_TamperPrechargeDuration_1RTCCLK: Tamper pins are pre-charged before sampling during 1 RTCCLK cycle
  *     @arg RTC_TamperPrechargeDuration_2RTCCLK: Tamper pins are pre-charged before sampling during 2 RTCCLK cycle
  *     @arg RTC_TamperPrechargeDuration_4RTCCLK: Tamper pins are pre-charged before sampling during 4 RTCCLK cycle
  *     @arg RTC_TamperPrechargeDuration_8RTCCLK: Tamper pins are pre-charged before sampling during 8 RTCCLK cycle
  * @retval None
  */
static inline void rtc_tamper_pins_precharge_duration(uint32_t RTC_TamperPrechargeDuration)
{
  /* Clear TAMPPRCH[1:0] bits in the RTC_TAFCR register */
  RTC->TAFCR &= (uint32_t)~(RTC_TAFCR_TAMPPRCH);

  /* Configure the RTC_TAFCR register */
  RTC->TAFCR |= (uint32_t)RTC_TamperPrechargeDuration;
}

/**
  * @brief  Enables or Disables the TimeStamp on Tamper Detection Event.
  * @note   The timestamp is valid even the TSE bit in tamper control register
  *         is reset.
  * @param  NewState: new state of the timestamp on tamper event.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rtc_time_stamp_on_tamper_detection_cmd(bool enable)
{

  if (enable)
  {
    /* Save timestamp on tamper detection event */
    RTC->TAFCR |= (uint32_t)RTC_TAFCR_TAMPTS;
  }
  else
  {
    /* Tamper detection does not cause a timestamp to be saved */
    RTC->TAFCR &= (uint32_t)~RTC_TAFCR_TAMPTS;
  }
}

/**
  * @brief  Enables or Disables the Precharge of Tamper pin.
  * @param  NewState: new state of tamper pull up.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rtc_tamper_pull_up_cmd(bool enable)
{

  if (enable)
  {
    /* Enable precharge of the selected Tamper pin */
    RTC->TAFCR &= (uint32_t)~RTC_TAFCR_TAMPPUDIS;
  }
  else
  {
    /* Disable precharge of the selected Tamper pin */
    RTC->TAFCR |= (uint32_t)RTC_TAFCR_TAMPPUDIS;
  }
}
#endif

/**
  * @brief  Writes a data in a specified RTC Backup data register.
  * @param  RTC_BKP_DR: RTC Backup data Register number.
  *          This parameter can be: RTC_BKP_DRx where x can be from 0 to 19 to
  *                          specify the register.
  * @param  Data: Data to be written in the specified RTC Backup data register.
  * @retval None
  */
static inline void rtc_write_backup_register(uint32_t RTC_BKP_DR, uint32_t Data)
{
  __IO uint32_t tmp = 0;

  tmp = RTC_BASE + 0x50;
  tmp += (RTC_BKP_DR * 4);

  /* Write the specified register */
  *(__IO uint32_t *)tmp = (uint32_t)Data;
}

/**
  * @brief  Reads data from the specified RTC Backup data Register.
  * @param  RTC_BKP_DR: RTC Backup data Register number.
  *          This parameter can be: RTC_BKP_DRx where x can be from 0 to 19 to
  *                          specify the register.
  * @retval None
  */
static inline uint32_t rtc_read_backup_register(uint32_t RTC_BKP_DR)
{
  __IO uint32_t tmp = 0;

  tmp = RTC_BASE + 0x50;
  tmp += (RTC_BKP_DR * 4);

  /* Read the specified register */
  return (*(__IO uint32_t *)tmp);
}

/**
  * @brief  Selects the RTC Tamper Pin.
  * @param  RTC_TamperPin: specifies the RTC Tamper Pin.
  *          This parameter can be one of the following values:
  *            @arg RTC_TamperPin_PC13: PC13 is selected as RTC Tamper Pin.
  *            @arg RTC_TamperPin_PI8: PI8 is selected as RTC Tamper Pin.
  * @retval None
  */
#ifdef RTC_TAFCR_TAMPINSEL
static inline void rtc_tamper_pin_selection(uint32_t RTC_TamperPin)
{

  RTC->TAFCR &= (uint32_t)~(RTC_TAFCR_TAMPINSEL);
  RTC->TAFCR |= (uint32_t)(RTC_TamperPin);
}
#endif

/**
  * @brief  Selects the RTC TimeStamp Pin.
  * @param  RTC_TimeStampPin: specifies the RTC TimeStamp Pin.
  *          This parameter can be one of the following values:
  *            @arg RTC_TimeStampPin_PC13: PC13 is selected as RTC TimeStamp Pin.
  *            @arg RTC_TimeStampPin_PI8: PI8 is selected as RTC TimeStamp Pin.
  * @retval None
  */
#ifdef RTC_TAFCR_TSINSEL
static inline void rtc_time_stamp_pin_selection(uint32_t RTC_TimeStampPin)
{

  RTC->TAFCR &= (uint32_t)~(RTC_TAFCR_TSINSEL);
  RTC->TAFCR |= (uint32_t)(RTC_TimeStampPin);
}
#endif

/**
  * @brief  Configures the RTC Output Pin mode.
  * @param  RTC_OutputType: specifies the RTC Output (PC13) pin mode.
  *          This parameter can be one of the following values:
  *            @arg RTC_OutputType_OpenDrain: RTC Output (PC13) is configured in
  *                                    Open Drain mode.
  *            @arg RTC_OutputType_PushPull:  RTC Output (PC13) is configured in
  *                                    Push Pull mode.
  * @retval None
  */
static inline void rtc_output_type_config(uint32_t RTC_OutputType)
{

  RTC->TAFCR &= (uint32_t)~(RTC_TAFCR_ALARMOUTTYPE);
  RTC->TAFCR |= (uint32_t)(RTC_OutputType);
}

#ifdef RTC_ISR_SHPF
/**
  * @brief  Configures the Synchronization Shift Control Settings.
  * @note   When REFCKON is set, firmware must not write to Shift control register
  * @param  RTC_ShiftAdd1S : Select to add or not 1 second to the time Calendar.
  *   This parameter can be one of the following values :
  *     @arg RTC_ShiftAdd1S_Set  : Add one second to the clock calendar.
  *     @arg RTC_ShiftAdd1S_Reset: No effect.
  * @param  RTC_ShiftSubFS: Select the number of Second Fractions to Substitute.
  *         This parameter can be one any value from 0 to 0x7FFF.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: RTC Shift registers are configured
  *          - ERROR: RTC Shift registers are not configured
*/
static inline bool rtc_synchro_shift_config(uint32_t RTC_ShiftAdd1S, uint32_t RTC_ShiftSubFS)
{
  uint32_t shpfcount = 0;

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  /* Check if a Shift is pending*/
  if ((RTC->ISR & RTC_ISR_SHPF) != RESET)
  {
    /* Wait until the shift is completed*/
    while (((RTC->ISR & RTC_ISR_SHPF) != RESET) && (shpfcount != SHPF_TIMEOUT))
    {
      shpfcount++;
    }
  }

  /* Check if the Shift pending is completed or if there is no Shift operation at all*/
  if ((RTC->ISR & RTC_ISR_SHPF) == RESET)
  {
    /* check if the reference clock detection is disabled */
    if((RTC->CR & RTC_CR_REFCKON) == RESET)
    {
      /* Configure the Shift settings */
      RTC->SHIFTR = (uint32_t)(uint32_t)(RTC_ShiftSubFS) | (uint32_t)(RTC_ShiftAdd1S);

      if(rtc_wait_for_synchro())
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return true;
    }
  }
  else
  {
    return true;
  }

  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;

  return true;
}

#endif

/**
  * @brief  Enables or disables the specified RTC interrupts.
  * @param  RTC_IT: specifies the RTC interrupt sources to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *            @arg RTC_IT_TS:  Time Stamp interrupt mask
  *            @arg RTC_IT_WUT:  WakeUp Timer interrupt mask
  *            @arg RTC_IT_ALRB:  Alarm B interrupt mask
  *            @arg RTC_IT_ALRA:  Alarm A interrupt mask
  *            @arg RTC_IT_TAMP: Tamper event interrupt mask
  * @param  NewState: new state of the specified RTC interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rtc_it_config(uint32_t RTC_IT, bool enable)
{

  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  if (enable)
  {
    /* Configure the Interrupts in the RTC_CR register */
    RTC->CR |= (uint32_t)(RTC_IT & ~RTC_TAFCR_TAMPIE);
    /* Configure the Tamper Interrupt in the RTC_TAFCR */
    RTC->TAFCR |= (uint32_t)(RTC_IT & RTC_TAFCR_TAMPIE);
  }
  else
  {
    /* Configure the Interrupts in the RTC_CR register */
    RTC->CR &= (uint32_t)~(RTC_IT & (uint32_t)~RTC_TAFCR_TAMPIE);
    /* Configure the Tamper Interrupt in the RTC_TAFCR */
    RTC->TAFCR &= (uint32_t)~(RTC_IT & RTC_TAFCR_TAMPIE);
  }
  /* Enable the write protection for RTC registers */
  RTC->WPR = 0xFF;
}

/**
  * @brief  Checks whether the specified RTC flag is set or not.
  * @param  RTC_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg RTC_FLAG_RECALPF: RECALPF event flag.
  *            @arg RTC_FLAG_TAMP1F: Tamper 1 event flag
  *            @arg RTC_FLAG_TSOVF: Time Stamp OverFlow flag
  *            @arg RTC_FLAG_TSF: Time Stamp event flag
  *            @arg RTC_FLAG_WUTF: WakeUp Timer flag
  *            @arg RTC_FLAG_ALRBF: Alarm B flag
  *            @arg RTC_FLAG_ALRAF: Alarm A flag
  *            @arg RTC_FLAG_INITF: Initialization mode flag
  *            @arg RTC_FLAG_RSF: Registers Synchronized flag
  *            @arg RTC_FLAG_INITS: Registers Configured flag
  *            @arg RTC_FLAG_SHPF: Shift operation pending flag.
  *            @arg RTC_FLAG_WUTWF: WakeUp Timer Write flag
  *            @arg RTC_FLAG_ALRBWF: Alarm B Write flag
  *            @arg RTC_FLAG_ALRAWF: Alarm A write flag
  * @retval The new state of RTC_FLAG (SET or RESET).
  */
static inline bool rtc_get_flag_status(uint32_t RTC_FLAG)
{
  uint32_t tmpreg = 0;

  /* Get all the flags */
  tmpreg = (uint32_t)(RTC->ISR & RTC_FLAGS_MASK);

  /* Return the status of the flag */
  return ((tmpreg & RTC_FLAG) != 0 );

}

/**
  * @brief  Clears the RTC's pending flags.
  * @param  RTC_FLAG: specifies the RTC flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg RTC_FLAG_TAMP1F: Tamper 1 event flag
  *            @arg RTC_FLAG_TSOVF: Time Stamp Overflow flag
  *            @arg RTC_FLAG_TSF: Time Stamp event flag
  *            @arg RTC_FLAG_WUTF: WakeUp Timer flag
  *            @arg RTC_FLAG_ALRBF: Alarm B flag
  *            @arg RTC_FLAG_ALRAF: Alarm A flag
  *            @arg RTC_FLAG_RSF: Registers Synchronized flag
  * @retval None
  */
static inline void rtc_clear_flag(uint32_t RTC_FLAG)
{
  /* Clear the Flags in the RTC_ISR register */
  RTC->ISR = (uint32_t)((uint32_t)(~((RTC_FLAG | RTC_ISR_INIT)& 0x0000FFFF) | (uint32_t)(RTC->ISR & RTC_ISR_INIT)));
}

/**
  * @brief  Checks whether the specified RTC interrupt has occurred or not.
  * @param  RTC_IT: specifies the RTC interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg RTC_IT_TS: Time Stamp interrupt
  *            @arg RTC_IT_WUT: WakeUp Timer interrupt
  *            @arg RTC_IT_ALRB: Alarm B interrupt
  *            @arg RTC_IT_ALRA: Alarm A interrupt
  *            @arg RTC_IT_TAMP1: Tamper 1 event interrupt
  * @retval The new state of RTC_IT (SET or RESET).
  */
static inline bool rtc_get_it_status(uint32_t RTC_IT)
{
  uint32_t tmpreg = 0, enablestatus = 0;


  /* Get the TAMPER Interrupt enable bit and pending bit */
  tmpreg = (uint32_t)(RTC->TAFCR & (RTC_TAFCR_TAMPIE));

  /* Get the Interrupt enable Status */
  enablestatus = (uint32_t)((RTC->CR & RTC_IT) | (tmpreg & (RTC_IT >> 15)));

  /* Get the Interrupt pending bit */
  tmpreg = (uint32_t)((RTC->ISR & (uint32_t)(RTC_IT >> 4)));

  /* Get the status of the Interrupt */
  return ((enablestatus != (uint32_t)0) && ((tmpreg & 0x0000FFFF) != (uint32_t)0));
}

/**
  * @brief  Clears the RTC's interrupt pending bits.
  * @param  RTC_IT: specifies the RTC interrupt pending bit to clear.
  *          This parameter can be any combination of the following values:
  *            @arg RTC_IT_TS: Time Stamp interrupt
  *            @arg RTC_IT_WUT: WakeUp Timer interrupt
  *            @arg RTC_IT_ALRB: Alarm B interrupt
  *            @arg RTC_IT_ALRA: Alarm A interrupt
  *            @arg RTC_IT_TAMP1: Tamper 1 event interrupt
  * @retval None
  */
static inline void rtc_clear_it_pending_bit(uint32_t RTC_IT)
{
  uint32_t tmpreg = 0;

  /* Get the RTC_ISR Interrupt pending bits mask */
  tmpreg = (uint32_t)(RTC_IT >> 4);

  /* Clear the interrupt pending bits in the RTC_ISR register */
  RTC->ISR = (uint32_t)((uint32_t)(~((tmpreg | RTC_ISR_INIT)& 0x0000FFFF) | (uint32_t)(RTC->ISR & RTC_ISR_INIT)));
}


#undef RTC_TR_RESERVED_MASK
#undef RTC_DR_RESERVED_MASK
#undef RTC_INIT_MASK
#undef RTC_RSF_MASK
#undef RECALPF_TIMEOUT
#undef SHPF_TIMEOUT
#undef INITMODE_TIMEOUT
#undef SYNCHRO_TIMEOUT


#ifdef __cplusplus
}
#endif /* __cplusplus */

