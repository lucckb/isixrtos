#ifndef _ISIX_USBH_INTERRUPT_H
#define _ISIX_USBH_INTERRUPT_H

#include <stm32system.h>
#include <usb/drivers/controllers/stm32/usb_config.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned usbh_irq_level_t;

void USBHovercurrentInterruptHandler(void);

static inline usbh_irq_level_t usbhp_get_interrupt_priority(void) {
	return CONFIG_USBHOST_USB_IRQ_MASK_VALUE;
}

static inline usbh_irq_level_t usbhp_protect_interrupt(void) {
	return irq_save( CONFIG_USBHOST_USB_IRQ_MASK_VALUE, 0 );
}

static inline void usbhp_unprotect_interrupt(usbh_irq_level_t level) {
	irq_restore( level );
}

#ifdef __cplusplus
}
#endif

#endif
