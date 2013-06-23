/*
 * usbbsp.c
 *
 *  Created on: 10-11-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <usb_bsp.h>
#include <isix.h>
#include <stm32system.h>
#include <stm32gpio.h>
#include <stm32rcc.h>
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif
/* ------------------------------------------------------------------ */
/* Stack define config */

#ifndef USB_STACK_IRQ_PRIO
#define USB_STACK_IRQ_PRIO 1
#endif

#ifndef USB_STACK_IRQ_SUBPRIO
#define USB_STACK_IRQ_SUBPRIO 6
#endif

/* ------------------------------------------------------------------ */
/* Setup USB board configuration */
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
	(void)pdev;
#ifdef USE_USB_OTG_FS
	gpio_clock_enable( GPIOA, true );
	gpio_abstract_config_ext( GPIOA, (1<<11)|(1<<12), AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_FULL );
	rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_OTG_FS, true );
#endif

#ifdef USE_USB_OTG_HS
#error USB HI speed not implemented yet
#endif
}
/* ------------------------------------------------------------------ */
void USB_OTG_BSP_uDelay (const uint32_t usec)
{
	for(uint32_t u = 120*usec/7; u>0; --u)
		nop();
}
/* ------------------------------------------------------------------ */
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
	isix_wait_ms( msec );
}

/* ------------------------------------------------------------------ */
void USB_OTG_BSP_EnableInterrupt (USB_OTG_CORE_HANDLE *pdev)
{
#ifdef USE_USB_OTG_HS
	nvic_set_priority(OTG_HS_IRQn, USB_STACK_IRQ_PRIO, USB_STACK_IRQ_SUBPRIO );
	nvic_irq_enable( OTG_HS_IRQn, true );
#else
	nvic_set_priority(OTG_FS_IRQn, USB_STACK_IRQ_PRIO, USB_STACK_IRQ_SUBPRIO );
	nvic_irq_enable( OTG_FS_IRQn, true );
#endif
#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
	nvic_set_priority(OTG_HS_EP1_OUT_IRQn, USB_STACK_IRQ_PRIO, USB_STACK_IRQ_SUBPRIO );
	nvic_set_priority(OTG_HS_EP1_IN_IRQn, USB_STACK_IRQ_PRIO, USB_STACK_IRQ_SUBPRIO );
	nvic_irq_enable( OTG_HS_EP1_OUT_IRQn, true );
	nvic_irq_enable( OTG_HS_EP1_IN_IRQn, true );
#endif
	(void)pdev;
}
/* ------------------------------------------------------------------ */
#ifdef USE_HOST_MODE
//TODO: Add specific version in host mode
/* ------------------------------------------------------------------ */
void USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{
	(void)pdev;
}
/* ------------------------------------------------------------------ */
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev,uint8_t state)
{
	(void)pdev;
	(void)state;
}
/* ------------------------------------------------------------------ */
#endif

/* ------------------------------------------------------------------ */


/* ------------------------------------------------------------------ */
