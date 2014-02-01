#include <stddef.h>
#include <stm32tim.h>
#include <stm32rcc.h>
#include <stm32system.h>
#include <usb/drivers/controllers/stm32/usb_config.h>
#include <usb/drivers/controllers/stm32/timer.h>
#include <usb/core/xcat.h>
#include <foundation/dbglog.h>
#include <isix.h>


// Isix milisec timer
static vtimer_t* isix_ms_timer;


/* Configure millisecond timer.
    prio    - interrupt preemption priority
    subprio - interrupt service priority when the same prio */
void TimerConfigure( unsigned prio, unsigned subprio, unsigned pclk1 ) 
{
	isix_ms_timer = isix_vtimer_create_oneshoot();
	if( !isix_ms_timer ) {
		isix_bug("FAILED");
	}
}

/* Configure millisecond timer to call a function in the future.
    timer   - timer number, from 1 to 4
    f       - function to call when time is elapsed
    time_ms - time to wait for, from 1 to 32767 milliseconds */
void TimerStart(int timer, void (*f)(void), unsigned time_ms) 
{
  	(void)timer;
	const int err = isix_vtimer_one_shoot(isix_ms_timer, (void (*)(void*))f, NULL, time_ms);
	if( err != ISIX_EOK ) {
		isix_bug("VTIMER one shoot");
	}
}

/* Stop time counting. The configured function will not be called.
    timer - timer number, from 1 to 4 */
void TimerStop(int timer) {
  (void)timer;
  const int err = isix_vtimer_stop(isix_ms_timer);
	if( err != ISIX_EOK ) {
		isix_bug("VTIMER one shoot");
	}
}

/* Make active delay.
    timer   - timer number, from 1 to 4
    time_ms - time to wait for. */
void ActiveWait(int timer, unsigned time_ms) {
	(void)timer;
	isix_wait_ms( time_ms );
}


#ifdef US_TIM_N

/** Microsecond timers **/

#define US_TIM             xcat(TIM, US_TIM_N)
#define US_TIM_IRQn        xcat3(TIM, US_TIM_N, _IRQn)
#define US_TIM_RCC         xcat(RCC_APB1Periph_TIM, US_TIM_N)
#define US_TIM_IRQHandler  xcat3(TIM, US_TIM_N, _IRQHandler)

static void (*us_callback1)(void) = NULL;
static void (*us_callback2)(void) = NULL;
static void (*us_callback3)(void) = NULL;
static void (*us_callback4)(void) = NULL;

/* Configure microsecond timer.
    prio    - interrupt preemption priority
    subprio - interrupt service priority when the same prio */
void FineTimerConfigure(unsigned prio, unsigned subprio, unsigned pclk1 )
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
void FineTimerStart(int timer, void (*f)(void), unsigned time_us)
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
void FineTimerStop(int timer) {
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
      us_callback1 = NULL;
      break;
    case 4:
      US_TIM->DIER &= ~TIM_IT_CC4;
      US_TIM->SR = ~TIM_IT_CC4;
      us_callback1 = NULL;
      break;
  }
}

/* The callback function is allowed to reenable a timer. */
void US_TIM_IRQHandler(void) {
  uint16_t it_status;
  void (*callback)(void);

  it_status = US_TIM->SR & US_TIM->DIER;
  if (it_status & TIM_IT_CC1) {
    callback = us_callback1;
    FineTimerStop(1);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC2) {
    callback = us_callback2;
    FineTimerStop(2);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC3) {
    callback = us_callback3;
    FineTimerStop(3);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC4) {
    callback = us_callback4;
    FineTimerStop(4);
    if (callback)
      callback();
  }
}
#endif /* US_TIM_N */
