/*
 * =====================================================================================
 *
 *       Filename:  ethernetif_prv.h
 *
 *    Description:  Ethernet if Private config
 *
 *        Version:  1.0
 *        Created:  30.08.2016 18:29:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdbool.h>


#pragma once

//Use interrupt instead of polling
#ifndef PHY_INT_USE_INTERRUPT
#define PHY_INT_USE_INTERRUPT 1
#endif
//Exti interrupt
#ifndef PHY_INT_EXTI_LINE_IRQ_N
#define PHY_INT_EXTI_LINE_IRQ_N EXTI15_10_IRQn
#endif
//Exti line num
#ifndef PHY_INT_EXTI_NUM
#define PHY_INT_EXTI_NUM 13
#endif
//Exti port
#ifndef PHY_INT_GPIO_PORT
#define PHY_INT_GPIO_PORT D
#endif
//Does use RMII interface
#ifndef ETH_DRV_USE_RMII
#define ETH_DRV_USE_RMII 0
#endif
//Configure clock for provide MCO
#ifndef ETH_DRV_CONFIGURE_MCO_OUTPUT
#define ETH_DRV_CONFIGURE_MCO_OUTPUT 0
#endif
//Driver interrupt priority and subpriority
#ifndef ETH_DRV_IRQ_PRIO
#define ETH_DRV_IRQ_PRIO 1
#endif
#ifndef ETH_DRV_IRQ_SUBPRIO
#define ETH_DRV_IRQ_SUBPRIO 7
#endif
//Operating system driver copy data priority
#ifndef ETH_DRV_ISIX_THREAD_PRIORITY
#define ETH_DRV_ISIX_THREAD_PRIORITY (isix_get_min_priority() - 1)
#endif
//SYSTEM HCLK for PHY emac speed calculation
#ifndef ETH_DRV_HCLK_HZ
#define ETH_DRV_HCLK_HZ CONFIG_HCLK_HZ
#endif
//PHY Address 1 for DP83848
#ifndef ETH_DRV_PHY_ADDR
#define ETH_DRV_PHY_ADDR 1
#endif

//
//Macros
#define PHY_INT_EXTI_CAT(x, y) x##y
#define PHY_INT_EXTI_XGPIO(X)PHY_INT_EXTI_CAT(GPIO, X)
#define PHY_INT_EXTI_XGPIOCAT(x, y) x##GPIO##y
#define PHY_INT_EXTI_LINENUM(x) 	 PHY_INT_EXTI_CAT(EXTI_Line , x )
#define PHY_INT_EXTI_PORT_SOURCE(x)  PHY_INT_EXTI_XGPIOCAT(GPIO_PortSource, x  )
#define PHY_INT_EXTI_PIN_SOURCE(x)   PHY_INT_EXTI_CAT(GPIO_PinSource, x  )



void _ethernetif_deinit_arch_(void);
void _ethernetif_gpio_mii_init_arch_(void);
void _ethernetif_gpio_rmii_init_arch_(void);
void _ethernetif_clock_setup_arch_(bool enable);
void _ethernetif_dma_setup_arch_(void);

//
