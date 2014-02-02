#include <usb/drivers/controllers/stm32/usb_interrupt.h>
#include <usb/drivers/controllers/stm32/usb_config.h>
#include <usb/drivers/controllers/stm32/timer.h>
#include <usb/drivers/controllers/stm32/usb_config.h>
#include <usb/core/xcat.h>

/** USB interrupt handlers for STM32F105, STM32F107, STM32F205,
    STM32F207, STM32F215, STM32F217, STM32F405, STM32F407, STM32F415
    and STM32F417 **/


void __attribute__((__interrupt__)) otg_fs_isr_vector( void ) {
  USBglobalInterruptHandler();
}

void __attribute__((__interrupt__)) otg_hs_isr_vector( void ) {
  USBglobalInterruptHandler();
}

#ifdef CONFIG_USBLIB_US_TIM_N
#define us_tim_isr_vector  usblib_xcat3(tim, CONFIG_USBLIB_US_TIM_N, _isr_vector)
void __attribute__((__interrupt__)) us_tim_isr_vector( void ) {
	usbh_prv_fine_timer_irq_handler();
}
#endif
