/*
 * board_boot.cpp
 *
 *  Created on: 5 gru 2013
 *      Author: lucck
 */

#include <isix.h>
#include <stm32lib.h>
#include <stm32rcc.h>
#include <stm32system.h>
/* ------------------------------------------------------------------ */
namespace {

	constexpr auto ISIX_NUM_PRIORITIES = 4;
}
/* ------------------------------------------------------------------ */
namespace {
	/** Cortex stm32 System setup
	 * Clock and flash configuration for selected rate
	 */
	void uc_periph_setup()
	{
		stm32::rcc_flash_latency( CONFIG_HCLK_HZ );
		stm32::rcc_pll1_sysclk_setup( stm32::e_sysclk_hse_pll, CONFIG_XTAL_HZ , CONFIG_HCLK_HZ );
		stm32::rcc_pclk2_config(  RCC_HCLK_Div2 );
		stm32::rcc_pclk1_config(  RCC_HCLK_Div4 );
		//Setup NVIC vector at begin of flash
		SCB->VTOR = NVIC_VectTab_FLASH;
	}
extern "C" {
	void _external_startup(void)
	{
		//Initialize system perhipheral
		uc_periph_setup();
		//1 bit for preemtion priority
		stm32::nvic_priority_group(NVIC_PriorityGroup_1);
		//System priorities
		stm32::nvic_set_priority(PendSV_IRQn,1,0x7);
		//System priorities
		stm32::nvic_set_priority(SVCall_IRQn,1,0x7);
		//Set timer priority
		stm32::nvic_set_priority(SysTick_IRQn,1,0x7);
		//Initialize isix
		isix::isix_init(ISIX_NUM_PRIORITIES);
		stm32::systick_config( isix::ISIX_HZ * (CONFIG_HCLK_HZ/(8000000U)) );
	}
}
/* ------------------------------------------------------------------ */
}


/* ------------------------------------------------------------------ */
