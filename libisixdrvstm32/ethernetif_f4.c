/*
 * =====================================================================================
 *
 *       Filename:  ethernetif_f4.c
 *
 *    Description:  Ethernet interface for stm32f4
 *
 *        Version:  1.0
 *        Created:  30.08.2016 19:46:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include "ethernetif_prv.h"
#include <stm32rcc.h>
#include <stm32system.h>


#if ETH_DRV_USE_RMII == 0
#error  MII inteface not implemented for selected platform
#endif

#define bv(x) (1<<(x))


/**
  * @brief  Deinitializes the ETHERNET peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */

void _ethernetif_deinit_arch_(void)
{
	rcc_ahb1_periph_reset_cmd( RCC_AHB1Periph_ETH_MAC, true );
	dmb();
	rcc_ahb1_periph_reset_cmd( RCC_AHB1Periph_ETH_MAC, false );
	dmb();
}


//! GPIO initialize MII setup
void _ethernetif_gpio_mii_init_arch_(void)
{
}


/** RMII ETH GPIO Configuration
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PB11     ------> ETH_TX_EN
    PB12     ------> ETH_TXD0
    PB13     ------> ETH_TXD1
    PC1     ------> ETH_MDC
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    */
void _ethernetif_gpio_rmii_init_arch_(void)
{
	rcc_ahb1_periph_clock_cmd( RCC_AHB1Periph_GPIOA|
			RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, true );
	rcc_apb2_periph_clock_cmd( RCC_APB2Periph_SYSCFG, true );
	//Configure alternate functions
	gpio_pin_AF_config( GPIOA, GPIO_PinSource1,  GPIO_AF_ETH );
	gpio_pin_AF_config( GPIOA, GPIO_PinSource2,  GPIO_AF_ETH );
	gpio_pin_AF_config( GPIOA, GPIO_PinSource7,  GPIO_AF_ETH );
	gpio_pin_AF_config( GPIOB, GPIO_PinSource11,  GPIO_AF_ETH );
	gpio_pin_AF_config( GPIOB, GPIO_PinSource12,  GPIO_AF_ETH );
	gpio_pin_AF_config( GPIOB, GPIO_PinSource13,  GPIO_AF_ETH );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource1,  GPIO_AF_ETH );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource4,  GPIO_AF_ETH );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource5,  GPIO_AF_ETH );
	gpio_config_ext( GPIOA, bv(1)|bv(2)|bv(7),
		GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP );
	gpio_config_ext( GPIOB, bv(11)|bv(12)|bv(13),
		GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP );
	gpio_config_ext( GPIOC, bv(1)|bv(4)|bv(5),
		GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP );
	//Configure PHY interrupt
	 gpio_config( PHY_INT_EXTI_XGPIO(PHY_INT_GPIO_PORT), PHY_INT_EXTI_NUM,
		GPIO_MODE_INPUT, GPIO_PUPD_NONE , 0 , 0 );
 	 gpio_eth_media_interface_config(GPIO_ETH_MediaInterface_RMII);
}


void _ethernetif_clock_setup_arch_(bool enable)
{
  rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
		RCC_AHB1Periph_ETH_MAC_Rx, enable );
}



void _ethernetif_dma_setup_arch_(void)
{

}
