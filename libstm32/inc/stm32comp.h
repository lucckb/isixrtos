/*
 * =====================================================================================
  * @file    stm32f37x_comp.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the comparators (COMP1 and COMP2) peripheral: 
  *           + Comparators configuration
  *           + Window mode control
  *
 *
 * =====================================================================================
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "stm32lib.h"



#define COMP_CSR_CLEAR_MASK              ((uint32_t)0x00003FFE)

#ifdef __cplusplus
namespace stm32 {
#endif

/**
  * @brief  Initializes the COMP peripheral according to the specified parameters
  *         in COMP_InitStruct
  * @note   If the selected comparator is locked, initialization can't be performed.
  *         To unlock the configuration, perform a system reset.
  * @note   By default, PA1 is selected as COMP1 non inverting input.
  *         To use PA4 as COMP1 non inverting input call COMP_SwitchCmd() after COMP_Init()
  * @param  COMP_Selection: the selected comparator. 
  *          This parameter can be one of the following values:
  *            @arg COMP_Selection_COMP1: COMP1 selected
  *            @arg COMP_Selection_COMP2: COMP2 selected
  * @param  COMP_InitStruct: pointer to an COMP_InitTypeDef structure that contains 
  *         the configuration information for the specified COMP peripheral.
  * @retval None
  */
static inline void comp_init(uint32_t comp_selection, uint32_t inverting_input,
		uint32_t output, uint32_t output_pol, uint32_t hysteresis, uint32_t mode )
{
  uint32_t tmpreg = 0;


  /*!< Get the COMP_CSR register value */
  tmpreg = COMP->CSR;

  /*!< Clear the COMP1SW1, COMPx_IN_SEL, COMPx_OUT_TIM_SEL, 
   * COMPx_POL, COMPx_HYST and COMPx_PWR_MODE bits */ 
  tmpreg &= (uint32_t) ~(COMP_CSR_CLEAR_MASK<<comp_selection);

  /*!< Configure COMP: inverting input, output redirection, hysteresis value and power mode */
  /*!< Set COMPxINSEL bits according to COMP_InitStruct->COMP_InvertingInput value */
  /*!< Set COMPxOUTSEL bits according to COMP_InitStruct->COMP_Output value */
  /*!< Set COMPxPOL bit according to COMP_InitStruct->COMP_OutputPol value */
  /*!< Set COMPxHYST bits according to COMP_InitStruct->COMP_Hysteresis value */
  /*!< Set COMPxMODE bits according to COMP_InitStruct->COMP_Mode value */ 
  tmpreg |= (uint32_t)((inverting_input | output |
                       output_pol | hysteresis | mode)<<comp_selection);

  /*!< Write to COMP_CSR register */
  COMP->CSR = tmpreg;  
}



/**
  * @brief  Deinitializes COMP peripheral registers to their default reset values.
  * @note   Deinitialization can't be performed if the COMP configuration is locked.
  *         To unlock the configuration, perform a system reset.
  * @param  None
  * @retval None
  */
static inline void comp_deinit(void)
{
  COMP->CSR = ((uint32_t)0x00000000);    /*!< Set COMP_CSR register to reset value */
}


/**
  * @brief  Enable or disable the COMP peripheral.
  * @note   If the selected comparator is locked, enable/disable can't be performed.
  *         To unlock the configuration, perform a system reset.
  * @param  COMP_Selection: the selected comparator.
  *          This parameter can be one of the following values:
  *            @arg COMP_Selection_COMP1: COMP1 selected
  *            @arg COMP_Selection_COMP2: COMP2 selected
  * @param  NewState: new state of the COMP peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   When enabled, the comparator compares the non inverting input with 
  *         the inverting input and the comparison result is available on comparator output.
  * @note   When disabled, the comparator doesn't perform comparison and the 
  *         output level is low.
  * @retval None
  */
static inline void comp_cmd(uint32_t COMP_Selection, bool enable )
{
  if ( enable )
  {
    /* Enable the selected COMP peripheral */
    COMP->CSR |= (uint32_t) (1<<COMP_Selection);
  }
  else
  {
    /* Disable the selected COMP peripheral  */
    COMP->CSR &= (uint32_t)(~((uint32_t)1<<COMP_Selection));
  }
}

/**
  * @brief  Close or Open the SW1 switch.
  * @note   This switch is solely intended to redirect signals onto high
  *         impedance input, such as COMP1 non-inverting input (highly resistive switch)
  * @param  NewState: New state of the analog switch.
  *          This parameter can be: ENABLE or DISABLE. 
  * @note   When enabled, the SW1 is closed; PA1 is connected to PA4
  * @note   When disabled, the SW1 switch is open; PA1 is disconnected from PA4
  * @retval None
  */
static inline void comp_switch_cmd( bool enable )
{
  if ( enable )
  {
    /* Close SW1 switch */
    COMP->CSR |= (uint32_t) (COMP_CSR_COMP1SW1);
  }
  else
  {
    /* Open SW1 switch */
    COMP->CSR &= (uint32_t)(~COMP_CSR_COMP1SW1);
  }
}

/**
  * @brief  Return the output level (high or low) of the selected comparator. 
  * @note   The output level depends on the selected polarity.
  * @note   If the polarity is not inverted:
  *          - Comparator output is low when the non-inverting input is at a lower
  *            voltage than the inverting input
  *          - Comparator output is high when the non-inverting input is at a higher
  *            voltage than the inverting input
  * @note   If the polarity is inverted:
  *          - Comparator output is high when the non-inverting input is at a lower
  *            voltage than the inverting input
  *          - Comparator output is low when the non-inverting input is at a higher
  *            voltage than the inverting input
  * @param  COMP_Selection: the selected comparator. 
  *          This parameter can be one of the following values:
  *            @arg COMP_Selection_COMP1: COMP1 selected
  *            @arg COMP_Selection_COMP2: COMP2 selected  
  * @retval Returns the selected comparator output level: low or high.
  *       
  */
static inline uint32_t comp_get_output_level(uint32_t COMP_Selection)
{
  uint32_t compout = 0x0;

  /* Check if selected comparator output is high */
  if ((COMP->CSR & (COMP_CSR_COMP1OUT<<COMP_Selection)) != 0)
  {
    compout = COMP_OutputLevel_High;
  }
  else
  {
    compout = COMP_OutputLevel_Low;
  }

  /* Return the comparator output level */
  return (uint32_t)(compout);
}

/**
  * @brief  Enables or disables the window mode.
  * @note   In window mode, COMP1 and COMP2 non inverting inputs are connected
  *         together and only COMP1 non inverting input (PA1) can be used.
  * @param  NewState: new state of the window mode.
  *          This parameter can be :
  *           @arg ENABLE: COMP1 and COMP2 non inverting inputs are connected together.
  *           @arg DISABLE: OMP1 and COMP2 non inverting inputs are disconnected.
  * @retval None
  */
static inline void comp_window_cmd( bool enable )
{
  if ( enable )
  {
    /* Enable the window mode */
    COMP->CSR |= (uint32_t) COMP_CSR_WNDWEN;
  }
  else
  {
    /* Disable the window mode */
    COMP->CSR &= (uint32_t)(~COMP_CSR_WNDWEN);
  }
}

/**
  * @brief  Lock the selected comparator (COMP1/COMP2) configuration.
  * @note   Locking the configuration means that all control bits are read-only.
  *         To unlock the comparator configuration, perform a system reset.
  * @param  COMP_Selection: selects the comparator to be locked 
  *          This parameter can be a value of the following values:
  *            @arg COMP_Selection_COMP1: COMP1 configuration is locked.
  *            @arg COMP_Selection_COMP2: COMP2 configuration is locked.  
  * @retval None
  */
static inline void comp_lock_config(uint32_t COMP_Selection)
{
  /* Set the lock bit corresponding to selected comparator */
  COMP->CSR |= (uint32_t) (COMP_CSR_COMP1LOCK<<COMP_Selection);
}


#ifdef __cplusplus
}
#endif

#undef COMP_CSR_CLEAR_MASK
