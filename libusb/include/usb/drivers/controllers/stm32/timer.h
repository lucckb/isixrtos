#ifndef _USBLIB_TIMER_H
#define _USBLIB_TIMER_H 1

#include <usb/drivers/controllers/stm32/usb_config.h>

#ifdef __cplusplus
extern "C" {
#endif

int TimerConfigure(void);
int TimerStart(unsigned, void (*)(void*), unsigned);
int TimerStop(unsigned);

/* Enabled only when the fine timer scheduler is enabled */
#ifdef CONFIG_USBLIB_US_TIM_N
void FineTimerConfigure(unsigned prio, unsigned subprio, unsigned pclk1 );
void FineTimerStart(int timer, void (*f)(void), unsigned time_us);
void FineTimerStop(int timer);
//! Private function called on fine timer handle
void usbh_prv_fine_timer_irq_handler( void );
#endif


#ifdef __cplusplus
}
#endif

#endif
