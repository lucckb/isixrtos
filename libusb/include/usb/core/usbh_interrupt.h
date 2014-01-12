#ifndef _USBH_INTERRUPT_H
#define _USBH_INTERRUPT_H 1


#ifdef __cplusplus
extern "C" {
#endif

//TODO: Fixme this
typedef unsigned irq_level_t;

void USBHovercurrentInterruptHandler(void);

static inline uint32_t USBHgetInterruptPriority(void) {
  //return MIDDLE_IRQ_PRIO;
  #warning todo3
}

static inline irq_level_t USBHprotectInterrupt(void) {
//  return IRQprotect(MIDDLE_IRQ_PRIO);
//  #warning todo1
}

static inline void USBHunprotectInterrupt(irq_level_t level) {
  //IRQunprotect(level);
  #warning todo2
}

#ifdef __cplusplus
}
#endif

#endif
