/* ---------------------------------------------------------------------------- */
/*
 * stm32_exti.h
 *
 *  Created on: 10-02-2012
 *      Author: lucck
 */
/* ---------------------------------------------------------------------------- */
#ifndef STM32_EXTI_H_
#define STM32_EXTI_H_
/* ---------------------------------------------------------------------------- */
#include "stm32f10x_exti.h"
#include <stdint.h>
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
 namespace stm32 {
#endif
/* ---------------------------------------------------------------------------- */
 /**
   * @brief  Initializes the EXTI peripheral according to the specified
   *         parameters in the EXTI_InitStruct.
   * @param  EXTI_InitStruct: pointer to a EXTI_InitTypeDef structure
   *         that contains the configuration information for the EXTI peripheral.
   * @retval None
   */

static inline void exti_init(uint32_t exti_line, EXTIMode_TypeDef exti_mode,
		EXTITrigger_TypeDef exti_trigger, bool line_enable )
{
  uint32_t tmp = 0;


  tmp = (uint32_t)EXTI_BASE;

  if ( line_enable )
  {
    /* Clear EXTI line configuration */
    EXTI->IMR &= ~exti_line;
    EXTI->EMR &= ~exti_line;

    tmp += exti_mode;

    *(__IO uint32_t *) tmp |= exti_line;

    /* Clear Rising Falling edge configuration */
    EXTI->RTSR &= ~exti_line;
    EXTI->FTSR &= ~exti_line;

    /* Select the trigger for the selected external interrupts */
    if (exti_trigger == EXTI_Trigger_Rising_Falling)
    {
      /* Rising Falling edge */
      EXTI->RTSR |= exti_line;
      EXTI->FTSR |= exti_line;
    }
    else
    {
      tmp = (uint32_t)EXTI_BASE;
      tmp += exti_trigger;

      *(__IO uint32_t *) tmp |= exti_line;
    }
  }
  else
  {
    tmp += exti_mode;

    /* Disable the selected external lines */
    *(__IO uint32_t *) tmp &= ~exti_line;
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Deinitializes the EXTI peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
static inline void exti_deinit(void)
{
  EXTI->IMR = 0x00000000;
  EXTI->EMR = 0x00000000;
  EXTI->RTSR = 0x00000000;
  EXTI->FTSR = 0x00000000;
  EXTI->PR = 0x000FFFFF;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Generates a Software interrupt.
  * @param  EXTI_Line: specifies the EXTI lines to be enabled or disabled.
  *   This parameter can be any combination of EXTI_Linex where x can be (0..19).
  * @retval None
  */
static inline void exti_generate_sw_interrupt(uint32_t EXTI_Line)
{
  EXTI->SWIER |= EXTI_Line;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the specified EXTI line flag is set or not.
  * @param  EXTI_Line: specifies the EXTI line flag to check.
  *   This parameter can be:
  *     @arg EXTI_Linex: External interrupt line x where x(0..19)
  * @retval The new state of EXTI_Line (SET or RESET).
  */
static inline bool exti_get_flag_status(uint32_t EXTI_Line)
{
  return (EXTI->PR & EXTI_Line)?true:false;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the specified EXTI line is asserted or not.
  * @param  EXTI_Line: specifies the EXTI line to check.
  *   This parameter can be:
  *     @arg EXTI_Linex: External interrupt line x where x(0..19)
  * @retval The new state of EXTI_Line (SET or RESET).
  */
static inline bool exti_get_it_status(uint32_t EXTI_Line)
{
  return ((EXTI->PR & EXTI_Line) && (EXTI->IMR & EXTI_Line))?true:false;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Clears the EXTI's line pending bits.
  * @param  EXTI_Line: specifies the EXTI lines to clear.
  *   This parameter can be any combination of EXTI_Linex where x can be (0..19).
  * @retval None
  */
static inline void exti_clear_it_pending_bit(uint32_t EXTI_Line)
{
  EXTI->PR = EXTI_Line;
}
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
}
 #endif
/* ---------------------------------------------------------------------------- */
#endif /* STM32_EXTI_H_ */
/* ---------------------------------------------------------------------------- */
