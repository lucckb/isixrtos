#ifndef _USBH_INTERRUPT_H
#define _USBH_INTERRUPT_H 1

#include <irq.h>

#ifdef __cplusplus
extern "C" {
#endif

void USBHovercurrentInterruptHandler(void);

static inline uint32_t USBHgetInterruptPriority(void) {
  return MIDDLE_IRQ_PRIO;
}

static inline irq_level_t USBHprotectInterrupt(void) {
  return IRQprotect(MIDDLE_IRQ_PRIO);
}

static inline void USBHunprotectInterrupt(irq_level_t level) {
  IRQunprotect(level);
}

#ifdef __cplusplus
}
#endif

#endif
