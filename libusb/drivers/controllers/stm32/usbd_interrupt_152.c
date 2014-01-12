#include <usb_interrupt.h>

#include <stm32l1xx_exti.h>

/** USB device interrupt handlers for STM32F151 and STM32F152 **/

/* Low-priority USB interrupt, triggered by all USB events */
void USB_LP_IRQHandler(void) {
  USBglobalInterruptHandler();
}

/* High-priority USB interrupt, triggered only by a correct transfer
   event for isochronous or double-buffered bulk transfer to reach the
   highest possible transfer rate */
void USB_HP_IRQHandler(void) {
}

/* Triggered by the wakeup event from the USB suspend mode */
void USB_FS_WKUP_IRQHandler(void) {
  EXTI_ClearITPendingBit(EXTI_Line18);
}
