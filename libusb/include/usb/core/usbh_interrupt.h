#ifndef _USBH_INTERRUPT_H
#define _USBH_INTERRUPT_H 1

#include <stm32system.h>

//TODO: FIX THIS

#ifdef __cplusplus
extern "C" {
#endif

//TODO: Fixme this
typedef unsigned irq_level_t;

void USBHovercurrentInterruptHandler(void);

static inline uint32_t USBHgetInterruptPriority(void) {
	return 1;
}

static inline irq_level_t USBHprotectInterrupt(void) {
	irq_mask( 1, 0 );
	return 0;
}

static inline void USBHunprotectInterrupt(irq_level_t level) {
	(void)level;
	irq_umask();
}

#ifdef __cplusplus
}
#endif

#endif
