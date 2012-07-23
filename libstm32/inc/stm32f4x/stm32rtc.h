/*
 * stm32rtc.h
 *
 *  Created on: 23-07-2012
 *      Author: lucck
 */
/* ---------------------------------------------------------------------------- */
#ifndef STM32F4RTC_H_
#define STM32F4RTC_H_
/* ---------------------------------------------------------------------------- */
#include "stm32f4xx_rtc.h"
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
namespace stm32 {
#endif

/* ---------------------------------------------------------------------------- */
/* Masks Definition */
#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)
#define RTC_DR_RESERVED_MASK    ((uint32_t)0x00FFFF3F)
#define RTC_INIT_MASK           ((uint32_t)0xFFFFFFFF)
#define RTC_RSF_MASK            ((uint32_t)0xFFFFFF5F)
#define INITMODE_TIMEOUT         ((uint32_t) 0x00010000)
#define SYNCHRO_TIMEOUT          ((uint32_t) 0x00020000)
#ifdef STM32MCU_MAJOR_TYPE_F4
#define RECALPF_TIMEOUT          ((uint32_t) 0x00020000)
#define SHPF_TIMEOUT             ((uint32_t) 0x00001000)
#else
#define RECALPF_TIMEOUT          ((uint32_t)0)
#define SHPF_TIMEOUT             ((uint32_t)0)
#endif

#define RTC_FLAGS_MASK          ((uint32_t)(RTC_FLAG_TSOVF | RTC_FLAG_TSF | RTC_FLAG_WUTF | \
                                            RTC_FLAG_ALRBF | RTC_FLAG_ALRAF | RTC_FLAG_INITF | \
                                            RTC_FLAG_RSF | RTC_FLAG_INITS | RTC_FLAG_WUTWF | \
                                            RTC_FLAG_ALRBWF | RTC_FLAG_ALRAWF | RTC_FLAG_TAMP1F | \
                                            RTC_FLAG_RECALPF | RTC_FLAG_SHPF))

/* ---------------------------------------------------------------------------- */
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
  if ((RTC->ISR & RTC_ISR_INITF) == (uint32_t)RESET)
  {
    /* Set the Initialization mode */
    RTC->ISR = (uint32_t)RTC_INIT_MASK;

    /* Wait till RTC is in INIT state and if Time out is reached exit */
    do
    {
      initstatus = RTC->ISR & RTC_ISR_INITF;
      initcounter++;
    } while((initcounter != INITMODE_TIMEOUT) && (initstatus == 0x00));

    if ((RTC->ISR & RTC_ISR_INITF) != RESET)
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    return false;
  }
}

/* ---------------------------------------------------------------------------- */
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

/* ---------------------------------------------------------------------------- */
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
/* ---------------------------------------------------------------------------- */
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
/* ---------------------------------------------------------------------------- */
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

  if ((RTC->ISR & RTC_ISR_RSF) != RESET)
  {
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
/* ---------------------------------------------------------------------------- */
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
/* ---------------------------------------------------------------------------- */
#ifdef STM32MCU_MAJOR_TYPE_F4
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
/* ---------------------------------------------------------------------------- */
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
    if ((RTC->CR & RTC_CR_FMT) != (uint32_t)RESET)
    {
    }
    else
    {
      H12 = 0x00;
    }
  }
  else
  {
    if ((RTC->CR & RTC_CR_FMT) != (uint32_t)RESET)
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
#ifdef STM32MCU_MAJOR_TYPE_F4
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
#ifdef STM32MCU_MAJOR_TYPE_F4
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
/* ---------------------------------------------------------------------------- */
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
/* ---------------------------------------------------------------------------- */
#ifdef STM32MCU_MAJOR_TYPE_F4
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
/* ---------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------- */
#undef RTC_TR_RESERVED_MASK
#undef RTC_DR_RESERVED_MASK
#undef RTC_INIT_MASK
#undef RTC_RSF_MASK
#undef RECALPF_TIMEOUT
#undef SHPF_TIMEOUT
#undef INITMODE_TIMEOUT
#undef SYNCHRO_TIMEOUT

/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif /* __cplusplus */
/* ---------------------------------------------------------------------------- */
#endif /* STM32RTC_H_ */
