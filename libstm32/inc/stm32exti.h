/*
 * stm32exti.h
 *
 *  Created on: 24-07-2012
 *      Author: lucck
 */

#pragma once


#include <stm32lib.h>


#ifdef __cplusplus
 namespace stm32 {
#endif



typedef enum EXTIMode_Type
{
  EXTI_Mode_Interrupt = 0x00,
  EXTI_Mode_Event = 0x04
} EXTIMode_TypeDef;

typedef enum EXTITrigger_Type
{
  EXTI_Trigger_Rising = 0x08,
  EXTI_Trigger_Falling = 0x0C,
  EXTI_Trigger_Rising_Falling = 0x10
} EXTITrigger_TypeDef;


 /**
   * @brief  Initializes the EXTI peripheral according to the specified
   *         parameters in the EXTI_InitStruct.
   * @param  EXTI_InitStruct: pointer to a EXTI_InitTypeDef structure
   *         that contains the configuration information for the EXTI peripheral.
   * @retval None
   */
 static inline void exti_init(uint32_t exti_line, EXTIMode_TypeDef exti_mode,
		 EXTITrigger_TypeDef exti_trigger, bool enable )
 {

	 uint32_t tmp = 0;


	 tmp = (uint32_t)EXTI_BASE;

	 if ( enable )
	 {
		 /* Clear EXTI line configuration */
		 *(__IO uint32_t *) (((uint32_t) &(EXTI->IMR)) + ((exti_line) >> 5 ) * 0x20) &= ~(uint32_t)(1 << (exti_line & 0x1F));
		 *(__IO uint32_t *) (((uint32_t) &(EXTI->EMR)) + ((exti_line) >> 5 ) * 0x20) &= ~(uint32_t)(1 << (exti_line & 0x1F));

		 tmp += exti_mode + (((exti_line) >> 5 ) * 0x20);

		 *(__IO uint32_t *) tmp |= (uint32_t)(1 << (exti_line & 0x1F));

		 tmp = (uint32_t)EXTI_BASE;

		 /* Clear Rising Falling edge configuration */
		 *(__IO uint32_t *) (((uint32_t) &(EXTI->RTSR)) + ((exti_line) >> 5 ) * 0x20) &= ~(uint32_t)(1 << (exti_line & 0x1F));
		 *(__IO uint32_t *) (((uint32_t) &(EXTI->FTSR)) + ((exti_line) >> 5 ) * 0x20) &= ~(uint32_t)(1 << (exti_line & 0x1F));

		 /* Select the trigger for the selected interrupts */
		 if (exti_trigger == EXTI_Trigger_Rising_Falling)
		 {
			 /* Rising Falling edge */
			 *(__IO uint32_t *) (((uint32_t) &(EXTI->RTSR)) + ((exti_line) >> 5 ) * 0x20) |= (uint32_t)(1 << (exti_line & 0x1F));
			 *(__IO uint32_t *) (((uint32_t) &(EXTI->FTSR)) + ((exti_line) >> 5 ) * 0x20) |= (uint32_t)(1 << (exti_line & 0x1F));
		 }
		 else
		 {
			 tmp += exti_trigger + (((exti_line) >> 5 ) * 0x20);

			 *(__IO uint32_t *) tmp |= (uint32_t)(1 << (exti_line & 0x1F));
		 }
	 }

	 else
	 {
		 tmp += exti_mode + (((exti_line) >> 5 ) * 0x20);

		 /* Disable the selected external lines */
		 *(__IO uint32_t *) tmp &= ~(uint32_t)(1 << (exti_line & 0x1F));
	 }
 }


/**
  * @brief  Deinitializes the EXTI peripheral registers to their default reset 
  *         values.
  * @param  None
  * @retval None
  */
static inline void exti_deinit(void)
{
  EXTI->IMR    = _STM32_EXTI_IMR_DEFVAL;
  EXTI->EMR    = 0x00000000;
  EXTI->RTSR   = 0x00000000;
  EXTI->FTSR   = 0x00000000;
  EXTI->SWIER  = 0x00000000;
  EXTI->PR     = _STM32_EXTI_PR_DEFVAL;
#ifdef _STM32_EXTI_IMR2_DEFVAL
  EXTI->IMR2   = _STM32_EXTI_IMR2_DEFVAL;
  EXTI->EMR2   = 0x00000000;
  EXTI->RTSR2  = 0x00000000;
  EXTI->FTSR2  = 0x00000000;
  EXTI->SWIER2 = 0x00000000;
  EXTI->PR2    = _STM32_EXTI_PR2_DEFVAL;
#endif
}



 /**
   * @brief  Generates a Software interrupt on selected EXTI line.
   * @param  EXTI_Line: specifies the EXTI line on which the software interrupt
   *         will be generated.
   *         This parameter can be any combination of EXTI_Linex where x can be (0..22)
   * @retval None
   */
 static inline void exti_generate_sw_interrupt(uint32_t EXTI_Line)
 {

  *(__IO uint32_t *) (((uint32_t) &(EXTI->SWIER)) + ((EXTI_Line) >> 5 ) * 0x20) |= (uint32_t)(1 << (EXTI_Line & 0x1F));
 }

 /**
   * @brief  Checks whether the specified EXTI line flag is set or not.
   * @param  EXTI_Line: specifies the EXTI line flag to check.
   *          This parameter can be EXTI_Linex where x can be(0..22)
   * @retval The new state of EXTI_Line (SET or RESET).
   */
 static inline bool exti_get_flag_status(uint32_t EXTI_Line)
 {

  return ((*(__IO uint32_t *) (((uint32_t) &(EXTI->PR)) + ((EXTI_Line) >> 5 ) * 0x20)& (uint32_t)(1 << (EXTI_Line & 0x1F))) != (uint32_t)0);
 }

 /**
   * @brief  Clears the EXTI's line pending flags.
   * @param  EXTI_Line: specifies the EXTI lines flags to clear.
   *          This parameter can be any combination of EXTI_Linex where x can be (0..22)
   * @retval None
   */
 static inline void exti_clear_flag(uint32_t EXTI_Line)
 {
  *(__IO uint32_t *) (((uint32_t) &(EXTI->PR)) + ((EXTI_Line) >> 5 ) * 0x20) = (1 << (EXTI_Line & 0x1F));
 }

 /**
   * @brief  Checks whether the specified EXTI line is asserted or not.
   * @param  EXTI_Line: specifies the EXTI line to check.
   *          This parameter can be EXTI_Linex where x can be(0..22)
   * @retval The new state of EXTI_Line (SET or RESET).
   */
 static inline bool exti_get_it_status(uint32_t EXTI_Line)
 {

  return ((*(__IO uint32_t *) (((uint32_t) &(EXTI->PR)) + ((EXTI_Line) >> 5 ) * 0x20)& (uint32_t)(1 << (EXTI_Line & 0x1F))) != (uint32_t)0);
 }

 /**
   * @brief  Clears the EXTI's line pending bits.
   * @param  EXTI_Line: specifies the EXTI lines to clear.
   *          This parameter can be any combination of EXTI_Linex where x can be (0..22)
   * @retval None
   */
 static inline void exti_clear_it_pending_bit(uint32_t EXTI_Line)
 {
	*(__IO uint32_t *) (((uint32_t) &(EXTI->PR)) + ((EXTI_Line) >> 5 ) * 0x20) = (1 << (EXTI_Line & 0x1F));
 }


#ifdef __cplusplus
}
#endif

