/*
 * =====================================================================================
 *
 *       Filename:  usb_config.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13.01.2014 21:50:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef  ISIX_USB_CONFIG_H_
#define  ISIX_USB_CONFIG_H_

#include <config/conf.h>

//! This option uses microsec timer for schedule host tasks
//#define CONFIG_USBLIB_US_TIM_N 4

//! Configure overcurrent pins
/* 
#define CONFIG_USBHOST_OVRCURR_ENABLE 1
#define CONFIG_USBHOST_OVRCURR_PIN_N   14
#define CONFIG_USBHOST_OVRCURR_GPIO_N  D
#define CONFIG_USBHOST_OVRCURR_EDGE EXTI_Trigger_Falling
#define CONFIG_USBHOST_OVRCURR_IRQ_N EXTI15_10
*/

//! Configure VBUS modef

//#define CONFIG_USBHOST_VBUS_GPIO_N  B
//#define CONFIG_USBHOST_VBUS_PIN   14
//#define CONFIG_USBHOST_VBUS_ON      1


//! Configure interrupt level
//#define CONFIG_USBHOST_USB_IRQ_MASK_VALUE 1

//Check the required configuration for ovr current
#ifdef CONFIG_USBHOST_OVRCURR_ENABLE
#ifndef  CONFIG_USBHOST_OVRCURR_PIN_N
#error Host overcurrent pin not defined
#endif
#ifndef CONFIG_USBHOST_OVRCURR_GPIO_N
#error Host overcurrent port not defined
#endif
#ifndef CONFIG_USBHOST_OVRCURR_EDGE
#error Host overcurrent edge not defined
#endif
#ifndef CONFIG_USBHOST_OVRCURR_IRQ_N 
#error Host overcurrent irqn not defined
#endif
#endif

#ifndef CONFIG_USBHOST_VBUS_GPIO_N 
#error CONFIG_USBHOST_VBUS_GPIO_N is not defined
#endif
#ifndef CONFIG_USBHOST_VBUS_PIN    
#error CONFIG_USBHOST_VBUS_PIN is not defined
#endif
#ifndef CONFIG_USBHOST_VBUS_GPIO_N 
#error CONFIG_USBHOST_VBUS_ON is not defined
#endif
#ifndef CONFIG_USBHOST_VBUS_OTYPE
#error  CONFIG_USBHOST_VBUS_OTYPE is not defined
#endif

#ifndef CONFIG_USBHOST_USB_IRQ_MASK_VALUE
#error CONFIG_USBHOST_USB_IRQ_MASK_VALUE is not defined
#endif

#endif   /* ----- #ifndef usb_config_INC  ----- */


