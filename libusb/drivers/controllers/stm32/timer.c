#include <stddef.h>
#include <stm32tim.h>
#include <stm32rcc.h>
#include <stm32system.h>
#include <usb/drivers/controllers/stm32/usb_config.h>
#include <usb/drivers/controllers/stm32/timer.h>
#include <usb/core/xcat.h>
#include <usb/core/usbh_error.h>
#include <foundation/sys/dbglog.h>
#include <isix.h>
#include <usb/drivers/controllers/stm32/usb_config.h>

// Isix milisec timer
static osvtimer_t isix_ms_timers[4];


/* Configure millisecond timer.
    prio    - interrupt preemption priority
    subprio - interrupt service priority when the same prio */
int usblibp_timer_configure( void ) 
{
	for( unsigned i=0; i<ARRAY_SIZE(isix_ms_timers); ++i ) {
		isix_ms_timers[i] = isix_vtimer_create();
		if( !isix_ms_timers[i] ) {
			return USBHLIB_ERROR_OS;
		}
	}
	return USBHLIB_SUCCESS;
}

/* Configure millisecond timer to call a function in the future.
    timer   - timer number, from 1 to 4
    f       - function to call when time is elapsed
    time_ms - time to wait for, from 1 to 32767 milliseconds */
int usblibp_timer_start(unsigned timer, void (*f)(void*), unsigned time_ms) 
{
	--timer;
	if( timer >= ARRAY_SIZE( isix_ms_timers ) ) {
		return USBHLIB_ERROR_NOT_SUPPORTED;
	}
	const int err = isix_vtimer_start_isr(isix_ms_timers[timer], f, 
			NULL, isix_ms2tick(time_ms), false );
	if( err != ISIX_EOK ) {
		return USBHLIB_ERROR_OS;
	}
	return USBHLIB_SUCCESS;
}

/* Stop time counting. The configured function will not be called.
    timer - timer number, from 1 to 4 */
int usblibp_timer_stop(unsigned timer) 
{
	--timer;
	if( timer >= ARRAY_SIZE( isix_ms_timers ) ) {
		return USBHLIB_ERROR_OS;
	}
    const int err = isix_vtimer_cancel_isr(isix_ms_timers[timer]);
	if( err != ISIX_EOK ) {
		return USBHLIB_ERROR_OS;
	}
	return USBHLIB_SUCCESS;
}


#ifdef CONFIG_USBLIB_US_TIM_N

/** Microsecond timers **/

#define US_TIM             usblib_xcat(TIM, CONFIG_USBLIB_US_TIM_N)
#define US_TIM_IRQn        usblib_xcat3(TIM, CONFIG_USBLIB_US_TIM_N, _IRQn)
#define US_TIM_RCC         usblib_xcat(RCC_APB1Periph_TIM, CONFIG_USBLIB_US_TIM_N)

static void (*us_callback1)(void) = NULL;
static void (*us_callback2)(void) = NULL;
static void (*us_callback3)(void) = NULL;
static void (*us_callback4)(void) = NULL;

/* Configure microsecond timer.
    prio    - interrupt preemption priority
    subprio - interrupt service priority when the same prio */
void usblibp_fine_timer_configure(unsigned prio, unsigned subprio, unsigned pclk1 )
{
	rcc_apb1_periph_clock_cmd( US_TIM_RCC, true );
	const unsigned  prescaler = pclk1 / 1000000 - 1;
	tim_timebase_init(US_TIM, prescaler, TIM_CounterMode_Up, 0xFFFF, TIM_CKD_DIV1, 0 );
	tim_oc_init( US_TIM, tim_cc_chn1, TIM_OCMode_Timing, 0, TIM_OutputState_Disable, 
			TIM_OutputNState_Disable, TIM_OCPolarity_High, TIM_OCNPolarity_High, 
			TIM_OCIdleState_Set, TIM_OCNIdleState_Reset
	);
	tim_oc_preload_config( US_TIM, tim_cc_chn1, false );
	nvic_set_priority( US_TIM_IRQn, prio, subprio );
	nvic_irq_enable( US_TIM_IRQn, true );
	tim_cmd( US_TIM, true );
}

/* Configure microsecond timer to call a function in the future.
    timer   - timer number, from 1 to 4
    f       - function to call when time is elapsed
    time_us - time to wait for, from 1 to 65535 microseconds */
void usblibp_fine_timer_start(int timer, void (*f)(void), unsigned time_us)
{
  switch (timer) {
    case 1:
      us_callback1 = f;
      US_TIM->CCR1 = US_TIM->CNT + time_us; /* 1 MHz */
      US_TIM->SR = ~TIM_IT_CC1;
      US_TIM->DIER |= TIM_IT_CC1;
      break;
    case 2:
      us_callback2 = f;
      US_TIM->CCR2 = US_TIM->CNT + time_us; /* 1 MHz */
      US_TIM->SR = ~TIM_IT_CC2;
      US_TIM->DIER |= TIM_IT_CC2;
      break;
    case 3:
      us_callback3 = f;
      US_TIM->CCR3 = US_TIM->CNT + time_us; /* 1 MHz */
      US_TIM->SR = ~TIM_IT_CC3;
      US_TIM->DIER |= TIM_IT_CC3;
      break;
    case 4:
      us_callback4 = f;
      US_TIM->CCR4 = US_TIM->CNT + time_us; /* 1 MHz */
      US_TIM->SR = ~TIM_IT_CC4;
      US_TIM->DIER |= TIM_IT_CC4;
      break;
  }
}

/* Stop time counting. The configured function will not be called.
    timer - timer number, from 1 to 4 */
void usblibp_fine_timer_stop(int timer) {
  switch (timer) {
    case 1:
      US_TIM->DIER &= ~TIM_IT_CC1;
      US_TIM->SR = ~TIM_IT_CC1;
      us_callback1 = NULL;
      break;
    case 2:
      US_TIM->DIER &= ~TIM_IT_CC2;
      US_TIM->SR = ~TIM_IT_CC2;
      us_callback2 = NULL;
      break;
    case 3:
      US_TIM->DIER &= ~TIM_IT_CC3;
      US_TIM->SR = ~TIM_IT_CC3;
      us_callback3 = NULL;
      break;
    case 4:
      US_TIM->DIER &= ~TIM_IT_CC4;
      US_TIM->SR = ~TIM_IT_CC4;
      us_callback4 = NULL;
      break;
  }
}

/* The callback function is allowed to reenable a timer. */
void usbh_prv_fine_timer_irq_handler(void) {
  uint16_t it_status;
  void (*callback)(void);

  it_status = US_TIM->SR & US_TIM->DIER;
  if (it_status & TIM_IT_CC1) {
    callback = us_callback1;
    usblibp_fine_timer_stop(1);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC2) {
    callback = us_callback2;
    usblibp_fine_timer_stop(2);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC3) {
    callback = us_callback3;
    usblibp_fine_timer_stop(3);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC4) {
    callback = us_callback4;
    usblibp_fine_timer_stop(4);
    if (callback)
      callback();
  }
}
#endif /* CONFIG_USBLIB_US_TIM_N */
