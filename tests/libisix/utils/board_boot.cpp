/*
 * board_boot.cpp
 *
 *  Created on: 5 gru 2013
 *      Author: lucck
 */

#include <isix.h>
#include <isix/arch/irq.h>
#include <isix/arch/irq.h>
#include <boot/arch/arm/cortexm/irq_vectors_table.h>
#include <stm32_ll_rcc.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_system.h>
#include <stm32_ll_pwr.h>
#include <cstdlib>
#include "stm32f4xx_ll_rcc.h"

namespace {
	/** Cortex stm32 System setup
	 * Clock and flash configuration for selected rate
	 */
#	if defined(STM32F411xE)
	bool uc_periph_setup()
	{
		constexpr auto retries=100000;
		isix_set_irq_vectors_base( &_exceptions_vectors );
		//! Deinitialize RCC
		LL_RCC_DeInit();
		LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
		LL_FLASH_EnablePrefetch();
		//! Set MCU Prescallers
		LL_RCC_SetAHBPrescaler( LL_RCC_SYSCLK_DIV_1 );
		LL_RCC_SetAPB2Prescaler( LL_RCC_APB2_DIV_1 );
		LL_RCC_SetAPB1Prescaler( LL_RCC_APB1_DIV_2 );
		//! Enable HSE generator
		LL_RCC_HSE_Enable();
		for( int i=0; i<retries; ++i ) {
			if(LL_RCC_HSE_IsReady()) {
				break;
			}
		}
		if( !LL_RCC_HSE_IsReady() ) {
			return false;
		}
		//Enable clocks for GPIOS
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA|LL_AHB1_GRP1_PERIPH_GPIOB|
				LL_AHB1_GRP1_PERIPH_GPIOC|LL_AHB1_GRP1_PERIPH_GPIOD|LL_AHB1_GRP1_PERIPH_GPIOE );

		//Configure PLL
		LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 400, LL_RCC_PLLP_DIV_4);
		LL_RCC_PLL_Enable();
		for( auto r=0; r<retries; ++r ) {
			if( LL_RCC_PLL_IsReady() ) {
				break;
			}
		}
		if( !LL_RCC_PLL_IsReady() ) {
			return false;
		}
		LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
		for( auto r=0; r<retries; ++r ) {
			if( LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL ) {
				break;
			}
		}
		return  LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL;
	}
#   elif defined(STM32F405xx)
	bool uc_periph_setup()
	{
		constexpr auto retries=10000;
		isix_set_irq_vectors_base( &_exceptions_vectors );
#ifndef QEMU_NO_RCC_PERIPH
		LL_RCC_DeInit();
#endif
        LL_RCC_HSI_Enable();
        LL_RCC_HSE_DisableBypass();
        LL_RCC_HSE_EnableCSS();
        //! Enable HSE generator
		LL_RCC_HSE_Enable();

		for( int i=0; i<retries; ++i ) {
			if(LL_RCC_HSE_IsReady()) {
				break;
			}
		}
#ifndef QEMU_NO_RCC_PERIPH
		if( !LL_RCC_HSE_IsReady() ) {
			return false;
		}
#endif
        LL_APB1_GRP1_EnableClock(RCC_APB1ENR_PWREN);
        LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

        //! Set MCU Prescallers
		LL_RCC_SetAHBPrescaler( LL_RCC_SYSCLK_DIV_1 );
		LL_RCC_SetAPB2Prescaler( LL_RCC_APB2_DIV_2 );
		LL_RCC_SetAPB1Prescaler( LL_RCC_APB1_DIV_4 );


		LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
		LL_FLASH_EnablePrefetch();

		//Enable clocks for GPIOS
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA|LL_AHB1_GRP1_PERIPH_GPIOB|
				LL_AHB1_GRP1_PERIPH_GPIOC|LL_AHB1_GRP1_PERIPH_GPIOD|LL_AHB1_GRP1_PERIPH_GPIOE );

		//Configure PLL
		LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_4, 168, LL_RCC_PLLP_DIV_2);
		LL_RCC_PLL_Enable();
		for( auto r=0; r<retries; ++r ) {
			if( LL_RCC_PLL_IsReady() ) {
				break;
			}
		}
#ifndef QEMU_NO_RCC_PERIPH
		if( !LL_RCC_PLL_IsReady() ) {
			return false;
		}
#endif
		LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
		for( auto r=0; r<retries; ++r ) {
			if( LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL ) {
				break;
			}
		}
        // Ignore qemu
#ifdef QEMU_NO_RCC_PERIPH
        return true;
#endif
		if( !LL_RCC_PLL_IsReady() ) {
			return  LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL;
		}
	}

#	else
#		error	Platform setup not defined
#	endif
extern "C" {
	void _external_startup(void)
	{
		//Initialize system perhipheral
		if( uc_periph_setup() ) {
			//1 bit voidfor preemtion priority
			isix_set_irq_priority_group( isix_cortexm_group_pri7 );
			//Initialize isix
			isix_init(CONFIG_HCLK_HZ);
		} else {
			for(;;);
		}
	}
}
}


