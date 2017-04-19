/*
 * board_boot.cpp
 *
 *  Created on: 5 gru 2013
 *      Author: lucck
 */

#include <isix.h>
#include <isix/arch/irq.h>
#include <stm32rcc.h>
#include <isix/arch/irq.h>
#include <irq_vectors_symbol.h>

namespace {
	/** Cortex stm32 System setup
	 * Clock and flash configuration for selected rate
	 */
	void uc_periph_setup()
	{
		stm32::rcc_flash_latency( CONFIG_HCLK_HZ );
		stm32::rcc_pll1_sysclk_setup( stm32::e_sysclk_hse_pll, CONFIG_XTAL_HZ , CONFIG_HCLK_HZ );
		stm32::rcc_pclk2_config( RCC_HCLK_Div2 );
		stm32::rcc_pclk1_config( RCC_HCLK_Div4 );
		//Setup NVIC vector at begin of flash
		isix_set_irq_vectors_base( &_exceptions_vectors );
	}
extern "C" {
	void _external_startup(void)
	{
		//Initialize system perhipheral
		uc_periph_setup();
		//1 bit for preemtion priority
		isix_set_irq_priority_group( isix_cortexm_group_pri7 );
		//Initialize isix
		isix_init(CONFIG_HCLK_HZ);
	}
}

}


