#include <board_usb_def.h>
#include <usb_interrupt.h>

/** USB interrupt handlers for STM32F105, STM32F107, STM32F205,
    STM32F207, STM32F215, STM32F217, STM32F405, STM32F407, STM32F415
    and STM32F417 **/

void OTG_FS_IRQHandler(void) {
  USBglobalInterruptHandler();
}

void OTG_HS_IRQHandler(void) {
  USBglobalInterruptHandler();
}

void OTG_FS_WKUP_IRQHandler(void) {
  EXTI_ClearITPendingBit(EXTI_Line18);
}

void OTG_HS_WKUP_IRQHandler(void) {
  EXTI_ClearITPendingBit(EXTI_Line18);
}
