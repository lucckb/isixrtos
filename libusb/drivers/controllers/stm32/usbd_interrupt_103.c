#include <usb_interrupt.h>

#include <stm32f10x_exti.h>

/** USB device interrupt handlers for STM32F102 and STM32F103 **/

/* Low-priority USB interrupt, triggered by all USB events */
void USB_LP_CAN1_RX0_IRQHandler(void) {
  USBglobalInterruptHandler();
}

/* High-priority USB interrupt, triggered only by a correct transfer
   event for isochronous or double-buffered bulk transfer to reach the
   highest possible transfer rate */
void USB_HP_CAN1_TX_IRQHandler(void) {
}

/* Triggered by the wakeup event from the USB suspend mode */
void USBWakeUp_IRQHandler(void) {
  EXTI_ClearITPendingBit(EXTI_Line18);
}
