/*
 * =====================================================================================
 *
 *       Filename:  stm32rcc.h
 *
 *    Description:  STM32RCC for F7 platform
 *
 *        Version:  1.0
 *        Created:  22.04.2017 16:03:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <stm32lib.h>


#ifdef __cplusplus
 namespace stm32 {
#endif



/** Setup the best flash latency according to the CPU speed
 * @param[in] frequency Sysclk frequency */
static inline void rcc_flash_latency(uint32_t frequency)
{
	uint32_t wait_states;

	wait_states = frequency / 30000000ul;	// calculate wait_states
	wait_states &= 0x0f;					// trim to max allowed value - 7
	// set wait_states, disable all caches and prefetch
	FLASH->ACR = wait_states;
	FLASH->ACR = FLASH_ACR_ARTRST | wait_states;	// reset caches
	// enable caches and prefetch
	FLASH->ACR = FLASH_ACR_ARTEN | FLASH_ACR_PRFTEN | wait_states;
}

/**
  * @brief  Configures the External High Speed oscillator (HSE).
  * @note   After enabling the HSE (RCC_HSE_ON or RCC_HSE_Bypass), the application
  *         software should wait on HSERDY flag to be set indicating that HSE clock
  *         is stable and can be used to clock the PLL and/or system clock.
  * @note   HSE state can not be changed if it is used directly or through the
  *         PLL as system clock. In this case, you have to select another source
  *         of the system clock then change the HSE state (ex. disable it).
  * @note   The HSE is stopped by hardware when entering STOP and STANDBY modes.
  * @note   This function reset the CSSON bit, so if the Clock security system(CSS)
  *         was previously enabled you have to enable it again after calling this
  *         function.
  * @param  RCC_HSE: specifies the new state of the HSE.
  *          This parameter can be one of the following values:
  *            @arg RCC_HSE_OFF: turn OFF the HSE oscillator, HSERDY flag goes low after
  *                              6 HSE oscillator clock cycles.
  *            @arg RCC_HSE_ON: turn ON the HSE oscillator
  *            @arg RCC_HSE_Bypass: HSE oscillator bypassed with external clock
  * @retval None
  */
enum rcc_hse_conf_e {
	RCC_HSE_OFF = 0,
	RCC_HSE_ON = RCC_CR_HSEON,
	RCC_HSE_BYPASS = (RCC_CR_HSEBYP | RCC_CR_HSEON)
};

static inline void rcc_hse_config(unsigned rcc_hse)
{

   RCC->CR &= ~0xf0000;
   RCC->CR |= rcc_hse;
}


/** @brief  Check RCC flag is set or not.
  * @param  RCC_FLAG: specifies the flag to check.
  *         This parameter can be one of the following values:
  *            @arg RCC_FLAG_HSIRDY: HSI oscillator clock ready.
  *            @arg RCC_FLAG_HSERDY: HSE oscillator clock ready.
  *            @arg RCC_FLAG_PLLRDY: Main PLL clock ready.
  *            @arg RCC_FLAG_PLLI2SRDY: PLLI2S clock ready.
  *            @arg RCC_FLAG_LSERDY: LSE oscillator clock ready.
  *            @arg RCC_FLAG_LSIRDY: LSI oscillator clock ready.
  *            @arg RCC_FLAG_BORRST: POR/PDR or BOR reset.
  *            @arg RCC_FLAG_PINRST: Pin reset.
  *            @arg RCC_FLAG_PORRST: POR/PDR reset.
  *            @arg RCC_FLAG_SFTRST: Software reset.
  *            @arg RCC_FLAG_IWDGRST: Independent Watchdog reset.
  *            @arg RCC_FLAG_WWDGRST: Window Watchdog reset.
  *            @arg RCC_FLAG_LPWRRST: Low Power reset.
  * @retval The new state of RCC_FLAG (TRUE or FALSE).
  */

/** Flag enconding
 *Elements values convention: 0XXYYYYYb
  *           - YYYYY  : Flag position in the register
  *           - 0XX  : Register index
  *                 - 01: CR register
  *                 - 10: BDCR register
  *                 - 11: CSR register
 */
enum rcc_flags_e {
	/* CR FLAGS */
	RCC_FLAG_HSIRDY                  = 0x21U,
	RCC_FLAG_HSERDY                  = 0x31U,
	RCC_FLAG_PLLRDY                  = 0x39U,
	RCC_FLAG_PLLI2SRDY               = 0x3BU,
	RCC_FLAG_PLLSAIRDY               = 0x3CU,
	/* BDCR FLAGS  */
	RCC_FLAG_LSERDY                  = 0x41U,
	/* CSR FLAGS */
	RCC_FLAG_LSIRDY                  = 0x61U,
	RCC_FLAG_BORRST                  = 0x79U,
	RCC_FLAG_PINRST                  = 0x7AU,
	RCC_FLAG_PORRST                  = 0x7BU,
	RCC_FLAG_SFTRST                  = 0x7CU,
	RCC_FLAG_IWDGRST                 = 0x7DU,
	RCC_FLAG_WWDGRST                 = 0x7EU,
	RCC_FLAG_LPWRRST                 = 0x7FU,

};

static inline bool rcc_get_flag_status(unsigned  RCC_FLAG)
{
	static const uint8_t RCC_FLAG_MASK = 0x1f;
    return (((((((RCC_FLAG) >> 5) == 1)?
			RCC->CR :((((RCC_FLAG) >> 5) == 2) ?
			RCC->BDCR :((((RCC_FLAG) >> 5) == 3)?
			RCC->CSR :RCC->CIR))) & ((uint32_t)1 << ((RCC_FLAG) & RCC_FLAG_MASK)))!= 0)
			? 1 : 0);
}



/**
  * @brief  Waits for HSE start-up.
  * @note   This functions waits on HSERDY flag to be set and return SUCCESS if
  *         this flag is set, otherwise returns ERROR if the timeout is reached
  *         and this flag is not set. The timeout value is defined by the constant
  *         HSE_STARTUP_TIMEOUT in stm32f4xx.h file. You can tailor it depending
  *         on the HSE crystal used in your application.
  * @param  None
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: HSE oscillator is stable and ready to use
  *          - ERROR: HSE oscillator not yet ready
  */
static inline bool rcc_wait_for_hse_startup(void)
{
	static const uint32_t HSE_STARTUP_TIMEOUT=0xffff;
	volatile uint32_t startupcounter = 0;
	bool hsestatus = false;
	/* Wait till HSE is ready and if Time out is reached exit */
	do
	{
		hsestatus = rcc_get_flag_status(RCC_FLAG_HSERDY);
		startupcounter++;
	} while((startupcounter != HSE_STARTUP_TIMEOUT) && (hsestatus == false));
	return rcc_get_flag_status( RCC_FLAG_HSERDY );
}


enum pll_source_e {
	RCC_PLLSource_HSI = RCC_PLLCFGR_PLLSRC_HSI,
	RCC_PLLSource_HSE = RCC_PLLCFGR_PLLSRC_HSE
};


enum pllp_e {
	RCC_PLLP_DIV2                   = 0x00000002U,
	RCC_PLLP_DIV4                   = 0x00000004U,
	RCC_PLLP_DIV6                   = 0x00000006U,
	RCC_PLLP_DIV8                   = 0x00000008U,

};


#if defined (RCC_PLLCFGR_PLLR)
/** @brief Macro to configure the main PLL clock source, multiplication and division factors.
  * @note   This function must be used only when the main PLL is disabled.
  * @param  __RCC_PLLSource__: specifies the PLL entry clock source.
  *         This parameter can be one of the following values:
  *            @arg RCC_PLLSOURCE_HSI: HSI oscillator clock selected as PLL clock entry
  *            @arg RCC_PLLSOURCE_HSE: HSE oscillator clock selected as PLL clock entry
  * @note   This clock source (RCC_PLLSource) is common for the main PLL and PLLI2S.  
  * @param  __PLLM__: specifies the division factor for PLL VCO input clock
  *         This parameter must be a number between Min_Data = 2 and Max_Data = 63.
  * @note   You have to set the PLLM parameter correctly to ensure that the VCO input
  *         frequency ranges from 1 to 2 MHz. It is recommended to select a frequency
  *         of 2 MHz to limit PLL jitter.
  * @param  __PLLN__: specifies the multiplication factor for PLL VCO output clock
  *         This parameter must be a number between Min_Data = 50 and Max_Data = 432.
  * @note   You have to set the PLLN parameter correctly to ensure that the VCO
  *         output frequency is between 100 and 432 MHz.
  * @param  __PLLP__: specifies the division factor for main system clock (SYSCLK)
  *         This parameter must be a number in the range {2, 4, 6, or 8}.
  * @note   You have to set the PLLP parameter correctly to not exceed 216 MHz on
  *         the System clock frequency.
  * @param  __PLLQ__: specifies the division factor for OTG FS, SDMMC and RNG clocks
  *         This parameter must be a number between Min_Data = 2 and Max_Data = 15.
  * @note   If the USB OTG FS is used in your application, you have to set the
  *         PLLQ parameter correctly to have 48 MHz clock for the USB. However,
  *         the SDMMC and RNG need a frequency lower than or equal to 48 MHz to work
  *         correctly.
  * @param  __PLLR__: specifies the division factor for DSI clock
  *         This parameter must be a number between Min_Data = 2 and Max_Data = 7.
  */

static inline void rcc_pll_config(uint32_t RCC_PLLSource, uint32_t PLLM,
		uint32_t PLLN, uint32_t PLLP, uint32_t PLLQ, uint32_t PLLR )
{
	RCC->PLLCFGR = (RCC_PLLSource | PLLM |
			(PLLN << POSITION_VAL(RCC_PLLCFGR_PLLN)) |
			(((PLLP >> 1) -1) << POSITION_VAL(RCC_PLLCFGR_PLLP)) |
			(PLLQ << POSITION_VAL(RCC_PLLCFGR_PLLQ)) |
			(PLLR << POSITION_VAL(RCC_PLLCFGR_PLLR)));
}
#else
/** @brief Macro to configure the main PLL clock source, multiplication and division factors.
  * @note   This function must be used only when the main PLL is disabled.
  * @param  __RCC_PLLSource__: specifies the PLL entry clock source.
  *         This parameter can be one of the following values:
  *            @arg RCC_PLLSOURCE_HSI: HSI oscillator clock selected as PLL clock entry
  *            @arg RCC_PLLSOURCE_HSE: HSE oscillator clock selected as PLL clock entry
  * @note   This clock source (RCC_PLLSource) is common for the main PLL and PLLI2S.  
  * @param  __PLLM__: specifies the division factor for PLL VCO input clock
  *         This parameter must be a number between Min_Data = 2 and Max_Data = 63.
  * @note   You have to set the PLLM parameter correctly to ensure that the VCO input
  *         frequency ranges from 1 to 2 MHz. It is recommended to select a frequency
  *         of 2 MHz to limit PLL jitter.
  * @param  __PLLN__: specifies the multiplication factor for PLL VCO output clock
  *         This parameter must be a number between Min_Data = 50 and Max_Data = 432.
  * @note   You have to set the PLLN parameter correctly to ensure that the VCO
  *         output frequency is between 100 and 432 MHz.
  * @param  __PLLP__: specifies the division factor for main system clock (SYSCLK)
  *         This parameter must be a number in the range {2, 4, 6, or 8}.
  * @note   You have to set the PLLP parameter correctly to not exceed 216 MHz on
  *         the System clock frequency.
  * @param  __PLLQ__: specifies the division factor for OTG FS, SDMMC and RNG clocks
  *         This parameter must be a number between Min_Data = 2 and Max_Data = 15.
  * @note   If the USB OTG FS is used in your application, you have to set the
  *         PLLQ parameter correctly to have 48 MHz clock for the USB. However,
  *         the SDMMC and RNG need a frequency lower than or equal to 48 MHz to work
  *         correctly.
  */
static inline void rcc_pll_config(uint32_t RCC_PLLSource, uint32_t PLLM,
		uint32_t PLLN, uint32_t PLLP, uint32_t PLLQ)
{
	RCC->PLLCFGR = (0x20000000 | RCC_PLLSource | PLLM |
			(PLLN << POSITION_VAL(RCC_PLLCFGR_PLLN)) |
			(((PLLP >> 1) -1) << POSITION_VAL(RCC_PLLCFGR_PLLP)) |
			(PLLQ << POSITION_VAL(RCC_PLLCFGR_PLLQ)));
}

#endif



/** @brief  Macros to enable or disable the main PLL.
  * @note   After enabling the main PLL, the application software should wait on 
  *         PLLRDY flag to be set indicating that PLL clock is stable and can
  *         be used as system clock source.
  * @note   The main PLL can not be disabled if it is used as system clock source
  * @note   The main PLL is disabled by hardware when entering STOP and STANDBY modes.
  */

static inline void rcc_pll_cmd( bool enable )
{
	if( enable ) SET_BIT(RCC->CR, RCC_CR_PLLON);
	else CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
}

//! RCC_APB1_APB2_Clock_Source RCC APB1/APB2 Clock Source
enum pclk_div_e {
		RCC_HCLK_Div1     =               RCC_CFGR_PPRE1_DIV1,
		RCC_HCLK_Div2     =               RCC_CFGR_PPRE1_DIV2,
		RCC_HCLK_Div4     =               RCC_CFGR_PPRE1_DIV4,
		RCC_HCLK_Div8     =               RCC_CFGR_PPRE1_DIV8,
		RCC_HCLK_Div16    =               RCC_CFGR_PPRE1_DIV16,
};

//! RCC_AHB_Clock_Source RCC AHB Clock Source
enum hclk_div_e {
	RCC_SYSCLK_Div1       =           RCC_CFGR_HPRE_DIV1,
	RCC_SYSCLK_Div2       =           RCC_CFGR_HPRE_DIV2,
	RCC_SYSCLK_Div4       =           RCC_CFGR_HPRE_DIV4,
	RCC_SYSCLK_Div8       =           RCC_CFGR_HPRE_DIV8,
	RCC_SYSCLK_Div16      =           RCC_CFGR_HPRE_DIV16,
	RCC_SYSCLK_Div64      =           RCC_CFGR_HPRE_DIV64,
	RCC_SYSCLK_Div128     =           RCC_CFGR_HPRE_DIV128,
	RCC_SYSCLK_Div256     =           RCC_CFGR_HPRE_DIV256,
	RCC_SYSCLK_Div512     =           RCC_CFGR_HPRE_DIV512,
};


static inline void rcc_hclk_config(uint32_t RCC_SYSCLK)
{
    MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK );
}


static inline void rcc_pclk1_config(uint32_t RCC_PCLK)
{

    MODIFY_REG( RCC->CFGR, RCC_CFGR_PPRE1, RCC_PCLK );
}



static inline void rcc_pclk2_config(uint32_t RCC_PCLK)
{
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, ((RCC_PCLK) << 3));
}



static inline void rcc_ahb1_periph_clock_cmd(uint32_t RCC_AHB1Periph, bool enable)
{
	if(enable) SET_BIT(RCC->AHB1ENR, RCC_AHB1Periph );
	else CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1Periph );
}



static inline void rcc_ahb2_periph_clock_cmd( uint32_t RCC_AHB2Periph, bool enable )
{
	if(enable) SET_BIT(RCC->AHB2ENR, RCC_AHB2Periph );
	else CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2Periph );
}



static inline void rcc_ahb3_periph_clock_cmd(uint32_t RCC_AHB3Periph, bool enable )
{

	if(enable) SET_BIT(RCC->AHB3ENR, RCC_AHB3Periph );
	else CLEAR_BIT(RCC->AHB3ENR, RCC_AHB3Periph );
}


static inline void rcc_apb1_periph_clock_cmd(uint32_t RCC_APB1Periph, bool enable )
{
	if(enable) SET_BIT(RCC->APB1ENR, RCC_APB1Periph );
	else CLEAR_BIT(RCC->APB1ENR, RCC_APB1Periph );
}



static inline void rcc_apb2_periph_clock_cmd(uint32_t RCC_APB2Periph, bool enable )
{
	if(enable) SET_BIT(RCC->APB2ENR, RCC_APB2Periph );
	else CLEAR_BIT(RCC->APB2ENR, RCC_APB2Periph );
}


/**
  * @brief  Enables or disables the I/O Compensation Cell.
  * @note   The I/O compensation cell can be used only when the device supply
  *         voltage ranges from 2.4 to 3.6 V.
  * @param  NewState: new state of the I/O Compensation Cell.
  *          This parameter can be one of the following values:
  *            @arg ENABLE: I/O compensation cell enabled
  *            @arg DISABLE: I/O compensation cell power-down mode
  * @retval None
  */
static inline void syscfg_compensation_cell_cmd( bool enabled )
{
	if(enabled) SET_BIT(SYSCFG->CMPCR, SYSCFG_CMPCR_CMP_PD);
	else CLEAR_BIT(SYSCFG->CMPCR, SYSCFG_CMPCR_CMP_PD);
}

enum sysclk_source_e {
	RCC_SYSCLKSource_HSI             = RCC_CFGR_SW_HSI,
	RCC_SYSCLKSource_HSE             = RCC_CFGR_SW_HSE,
	RCC_SYSCLKSource_PLLCLK          = RCC_CFGR_SW_PLL,
};

/**
  * @brief  Configures the system clock (SYSCLK).
  * @param  RCC_SYSCLKSource: specifies the clock source used as system clock.
  *   This parameter can be one of the following values:
  *     @arg RCC_SYSCLKSource_HSI: HSI selected as system clock
  *     @arg RCC_SYSCLKSource_HSE: HSE selected as system clock
  *     @arg RCC_SYSCLKSource_PLLCLK: PLL selected as system clock
  * @retval None
  */
static inline void rcc_sysclk_config(uint32_t RCC_SYSCLKSource)
{
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_SYSCLKSource );
}



static inline uint8_t rcc_get_sysclk_source(void)
{
	return ((uint32_t)(RCC->CFGR & RCC_CFGR_SWS));
}


/**
  * @brief  Selects the clock source to output on MCO1 pin(PA8).
  * @note   PA8 should be configured in alternate function mode.
  * @param  RCC_MCO1Source: specifies the clock source to output.
  *          This parameter can be one of the following values:
  *            @arg RCC_MCO1Source_HSI: HSI clock selected as MCO1 source
  *            @arg RCC_MCO1Source_LSE: LSE clock selected as MCO1 source
  *            @arg RCC_MCO1Source_HSE: HSE clock selected as MCO1 source
  *            @arg RCC_MCO1Source_PLLCLK: main PLL clock selected as MCO1 source
  * @param  RCC_MCO1Div: specifies the MCO1 prescaler.
  *          This parameter can be one of the following values:
  *            @arg RCC_MCO1Div_1: no division applied to MCO1 clock
  *            @arg RCC_MCO1Div_2: division by 2 applied to MCO1 clock
  *            @arg RCC_MCO1Div_3: division by 3 applied to MCO1 clock
  *            @arg RCC_MCO1Div_4: division by 4 applied to MCO1 clock
  *            @arg RCC_MCO1Div_5: division by 5 applied to MCO1 clock
  * @retval None
  */
enum mco01_source_e {
	RCC_MCO1Source_HSI       =        0,
	RCC_MCO1Source_LSE       =        RCC_CFGR_MCO1_0,
	RCC_MCO1Source_HSE       =        RCC_CFGR_MCO1_1,
	RCC_MCO1Source_PLLCLK    =        RCC_CFGR_MCO1,
};

enum mco01_div_e {

	RCC_MCO1Div_1  =                  0,
	RCC_MCO1Div_2  =                  RCC_CFGR_MCO1PRE_2,
	RCC_MCO1Div_3  =                  ((uint32_t)RCC_CFGR_MCO1PRE_0 | RCC_CFGR_MCO1PRE_2),
	RCC_MCO1Div_4  =                  ((uint32_t)RCC_CFGR_MCO1PRE_1 | RCC_CFGR_MCO1PRE_2),
	RCC_MCO1Div_5  =                  RCC_CFGR_MCO1PRE,
};


static inline void rcc_mco1_config(uint32_t RCC_MCO1Source, uint32_t RCC_MCO1Div)
{
	MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE),
			(RCC_MCO1Source |RCC_MCO1Div) );
}


#ifdef __cplusplus
 }
#endif


