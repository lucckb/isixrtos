/*
 * =====================================================================================
 *
 *       Filename:  ethernetif_f1.c
 *
 *    Description:  Ethernetif F1 cfg
 *
 *        Version:  1.0
 *        Created:  30.08.2016 18:25:24
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



#if ETH_DRV_USE_RMII == 1
#error  MII inteface not implemented for selected platform
#endif

//
/**
  * @brief  Deinitializes the ETHERNET peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */

void _ethernetif_deinit_arch_(void)
{

  rcc_ahb_periph_reset_cmd( RCC_AHBPeriph_ETH_MAC, true );
  dmb();
  rcc_ahb_periph_reset_cmd( RCC_AHBPeriph_ETH_MAC, false );
  dmb();
}



//! GPIO initialize MII setup
void _ethernetif_gpio_mii_init_arch_(void)
{
    //Enable gpios
	rcc_apb2_periph_clock_cmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
		RCC_APB2Periph_GPIOC |	RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO, true);
	/* ETHERNET pins configuration */
	  /* AF Output Push Pull:
	  - ETH_MII_MDIO / ETH_RMII_MDIO: PA2
	  - ETH_MII_MDC / ETH_RMII_MDC: PC1
	  - ETH_MII_TXD2: PC2
	  - ETH_MII_TX_EN / ETH_RMII_TX_EN: PB11
	  - ETH_MII_TXD0 / ETH_RMII_TXD0: PB12
	  - ETH_MII_TXD1 / ETH_RMII_TXD1: PB13
	  - ETH_MII_PPS_OUT / ETH_RMII_PPS_OUT: PB5
	  - ETH_MII_TXD3: PB8 */

	  /* Configure PA2 as alternate function push-pull */
	  gpio_config( GPIOA, 2,  GPIO_MODE_50MHZ, GPIO_CNF_ALT_PP );
	  /* Configure PC1, PC2 and PC3 as alternate function push-pull */
	  gpio_config_ext( GPIOC, (1<<1)| (1<<2), GPIO_MODE_50MHZ, GPIO_CNF_ALT_PP );
	  /* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
	  gpio_config_ext( GPIOB, (1<<5)|(1<<8)|(1<<11)|(1<<12)|(1<<13), GPIO_MODE_50MHZ, GPIO_CNF_ALT_PP );
	  /*               For Remapped Ethernet pins                   */
	  /* Input (Reset Value):
	  - ETH_MII_CRS CRS: PA0
	  - ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1
	  - ETH_MII_COL: PA3
	  - ETH_MII_RX_DV / ETH_RMII_CRS_DV: PD8
	  - ETH_MII_TX_CLK: PC3
	  - ETH_MII_RXD0 / ETH_RMII_RXD0: PD9
	  - ETH_MII_RXD1 / ETH_RMII_RXD1: PD10
	  - ETH_MII_RXD2: PD11
	  - ETH_MII_RXD3: PD12
	  - ETH_MII_RX_ER: PB10 */

	  /* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
	  gpio_pin_remap_config(GPIO_Remap_ETH, true);

	  /* Configure PA0, PA1 and PA3 as input */
	  gpio_config_ext(GPIOA, (1<<0)|(1<<1)|(1<<3), GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  /* Configure PB10 as input */
	  gpio_config( GPIOB, 10, GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  /* Configure PC3 as input */
	  gpio_config( GPIOC, 3, GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  /* Configure PD8, PD9, PD10, PD11 and PD12 as input */
	  gpio_config_ext(GPIOD, (1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12), GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  /* MCO pin configuration */
	  /* Configure MCO (PA8) as alternate function push-pull */
#if  ETH_DRV_CONFIGURE_MCO_OUTPUT
	  gpio_config( GPIOA, 8 , GPIO_MODE_50MHZ, GPIO_CNF_ALT_PP);
#endif
	  //Configure interrupt PHY port
	  gpio_config( PHY_INT_EXTI_XGPIO(PHY_INT_GPIO_PORT), PHY_INT_EXTI_NUM, GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  //Mapping
	 gpio_eth_media_interface_config(GPIO_ETH_MediaInterface_MII);
}



void _ethernetif_gpio_rmii_init_arch_(void)
{
	//TODO fill it
  	 gpio_eth_media_interface_config(GPIO_ETH_MediaInterface_RMII);
}



//! Initialize clocks
void _ethernetif_clock_setup_arch_(bool enable)
{
  rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
			RCC_AHBPeriph_ETH_MAC_Rx, enable );
}



/*
* Description
If a WFI/WFE instruction is executed to put the system in sleep mode while the Ethernet
MAC master clock on the AHB bus matrix is ON and all remaining masters clocks are OFF,
the Ethernet DMA will be not able to perform any AHB master accesses during sleep mode.
Workaround
Enable DMA1 or DMA2 clocks in the RCC_AHBENR register before executing the
WFI/WFE instruction.
*
*/
void _ethernetif_dma_setup_arch_(void)
{
	rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_DMA2, true );
}
