/*
 * stm32rcc.h
 *
 *  Created on: 19-07-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef STM32F4_STM32RCC_H_
#define STM32F4_STM32RCC_H_
/* ------------------------------------------------------------------ */
#include "stm32f4xx.h"
/* ------------------------------------------------------------------ */
#ifdef __cplusplus
 namespace stm32 {
#endif

/* ------------------------------------------------------------------ */
/** Setup the best flash latency according to the CPU speed */
static inline void rcc_flash_latency(uint32_t frequency)
{
	uint32_t wait_states;

	wait_states = frequency / 30000000ul;	// calculate wait_states (30M is valid for 2.7V to 3.6V voltage range, use 24M for 2.4V to 2.7V, 18M for 2.1V to 2.4V or 16M for  1.8V to 2.1V)
	wait_states &= 7;						// trim to max allowed value - 7
	FLASH->ACR = wait_states;				// set wait_states, disable all caches and prefetch
	FLASH->ACR = FLASH_ACR_DCRST | FLASH_ACR_ICRST | wait_states;	// reset caches
	FLASH->ACR = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | wait_states;	// enable caches and prefetch
}

/* ------------------------------------------------------------------ */
enum e_sysclk_mode
{
	e_sysclk_hsi,
	e_sysclk_hse,
	e_sysclk_hsi_pll,
	e_sysclk_hse_pll
};

#ifndef __cplusplus
#define RCC_PLLCFGR_PLLM_bit                            0
#define RCC_PLLCFGR_PLLN_bit                            6
#define RCC_PLLCFGR_PLLP_bit                            16
#define RCC_PLLCFGR_PLLQ_DIV9_value                     9
#define RCC_PLLCFGR_PLLQ_DIV9                           (RCC_PLLCFGR_PLLQ_DIV9_value << RCC_PLLCFGR_PLLQ_bit)
#define RCC_PLLCFGR_PLLQ_bit                            24
#endif

static inline uint32_t rcc_pll1_sysclk_setup(enum e_sysclk_mode mode, uint32_t crystal, uint32_t frequency)
{
#ifdef __cplusplus
	static const uint32_t RCC_PLLCFGR_PLLM_bit          =                  0;
	static const uint32_t RCC_PLLCFGR_PLLN_bit          =                  6;
	static const uint32_t RCC_PLLCFGR_PLLP_bit          =                  16;
	static const uint32_t RCC_PLLCFGR_PLLQ_DIV9_value   =                  9;
	static const uint32_t RCC_PLLCFGR_PLLQ_bit          =                  24;
	static const uint32_t RCC_PLLCFGR_PLLQ_DIV9         =   RCC_PLLCFGR_PLLQ_DIV9_value << RCC_PLLCFGR_PLLQ_bit;
#endif
	uint32_t best_frequency_core = 0;
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hse )
		RCC->CR  |= RCC_CR_HSEON;
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hsi_pll )
	{
		uint32_t div, mul, div_core, vco_input_frequency, vco_output_frequency, frequency_core;
		uint32_t best_div = 0, best_mul = 0, best_div_core = 0;
		for (div = 2; div <= 63; div++)			// PLLM in [2; 63]
		{
			vco_input_frequency = crystal / div;

			if ((vco_input_frequency < 1000000ul) || (vco_input_frequency > 2000000))	// skip invalid settings
				continue;

			for (mul = 64; mul <= 432; mul++)	// PLLN in [64; 432]
			{
				vco_output_frequency = vco_input_frequency * mul;

				if ((vco_output_frequency < 64000000ul) || (vco_output_frequency > 432000000ul))	// skip invalid settings
					continue;

				for (div_core = 2; div_core <= 8; div_core += 2)	// PLLP in {2, 4, 6, 8}
				{
					frequency_core = vco_output_frequency / div_core;

					if (frequency_core > frequency)	// skip values over desired frequency
						continue;

					if (frequency_core > best_frequency_core)	// is this configuration better than previous one?
					{
						best_frequency_core = frequency_core;	// yes - save values
						best_div = div;
						best_mul = mul;
						best_div_core = div_core;
					}
				}
			}
		}
	   // configure PLL factors, always divide USB clock by 9
		RCC->PLLCFGR = (best_div << RCC_PLLCFGR_PLLM_bit) | (best_mul << RCC_PLLCFGR_PLLN_bit) |
				((best_div_core / 2 - 1) << RCC_PLLCFGR_PLLP_bit)
				| RCC_PLLCFGR_PLLQ_DIV9 | (mode == e_sysclk_hse_pll?RCC_PLLCFGR_PLLSRC_HSE:RCC_PLLCFGR_PLLSRC_HSI);
		// AHB - no prescaler, APB1 - divide by 16, APB2 - divide by 16 (always safe value)
		RCC->CFGR = RCC_CFGR_PPRE2_DIV16 | RCC_CFGR_PPRE1_DIV16 | RCC_CFGR_HPRE_DIV1;
	}
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hse ) while(1)
	{
	   if(RCC->CR & RCC_CR_HSERDY) break;
	}
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hsi_pll )
	{
		RCC->CR |= RCC_CR_PLLON;
		while(1)
		{
			if(RCC->CR & RCC_CR_PLLRDY) break;
		}
		RCC->CFGR |= RCC_CFGR_SW_PLL;			// change SYSCLK to PLL
		while (((RCC->CFGR) & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	// wait for switch
	}
	else if( mode == e_sysclk_hsi )
	{
		RCC->CFGR |= RCC_CFGR_SW_HSI;			// change SYSCLK to PLL
		while ( ((RCC->CFGR) & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI );	// wait for switch
		RCC->CR &= ~RCC_CR_PLLON;
		best_frequency_core = 16000000ul;
	}
	else if( mode == e_sysclk_hse )
	{
		RCC->CFGR |= RCC_CFGR_SW_HSE;			// change SYSCLK to PLL
		while ( ((RCC->CFGR) & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE );	// wait for switch
		RCC->CR &= ~RCC_CR_PLLON;
		best_frequency_core = crystal;
	}
	return best_frequency_core;
}
#ifndef __cplusplus
#undef RCC_PLLCFGR_PLLM_bit
#undef RCC_PLLCFGR_PLLN_bit
#undef RCC_PLLCFGR_PLLP_bit
#undef RCC_PLLCFGR_PLLQ_DIV9_value
#undef RCC_PLLCFGR_PLLQ_DIV9
#undef RCC_PLLCFGR_PLLQ_bit
#endif

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
 }
#endif
/* ------------------------------------------------------------------ */
#endif /* STM32RCC_H_ */
