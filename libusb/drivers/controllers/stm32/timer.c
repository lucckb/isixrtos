#include <stddef.h>
#include <stm32tim.h>
#include <stm32rcc.h>
#include <stm32system.h>
#include <usb/drivers/controllers/stm32/usb_config.h>
#include <usb/drivers/controllers/stm32/timer.h>
#include <usb/core/xcat.h>
#include <foundation/dbglog.h>


#ifdef MS_TIM_N
/** Millisecond timers **/

#define MS_TIM             xcat(TIM, MS_TIM_N)
#define MS_TIM_IRQn        xcat3(TIM, MS_TIM_N, _IRQn)
#define MS_TIM_RCC         xcat(RCC_APB1Periph_TIM, MS_TIM_N)
#define MS_TIM_IRQHandler  xcat3(TIM, MS_TIM_N, _IRQHandler)

static void (*ms_callback1)(void) = NULL;
static void (*ms_callback2)(void) = NULL;
static void (*ms_callback3)(void) = NULL;
static void (*ms_callback4)(void) = NULL;


/* Configure millisecond timer.
    prio    - interrupt preemption priority
    subprio - interrupt service priority when the same prio */
void TimerConfigure(unsigned prio, unsigned subprio, unsigned pclk1 ) 
{
	rcc_apb1_periph_clock_cmd( MS_TIM_RCC, true );
	const unsigned  prescaler = pclk1 / 1000 - 1;
	tim_timebase_init(MS_TIM, prescaler, TIM_CounterMode_Up, 0xFFFF, TIM_CKD_DIV1, 0 );
	tim_oc_init( MS_TIM, tim_cc_chn1, TIM_OCMode_Timing, 0, TIM_OutputState_Disable, 
			TIM_OutputNState_Disable, TIM_OCPolarity_High, TIM_OCNPolarity_High, 
			TIM_OCIdleState_Set, TIM_OCNIdleState_Reset
	);
	tim_oc_preload_config( MS_TIM, tim_cc_chn1, false );
	nvic_set_priority( MS_TIM_IRQn, prio, subprio );
	nvic_irq_enable( MS_TIM_IRQn, true );
	tim_cmd( MS_TIM, true );
}

/* Configure millisecond timer to call a function in the future.
    timer   - timer number, from 1 to 4
    f       - function to call when time is elapsed
    time_ms - time to wait for, from 1 to 32767 milliseconds */
void TimerStart(int timer, void (*f)(void), unsigned time_ms) {
  switch (timer) {
    case 1:
      ms_callback1 = f;
      MS_TIM->CCR1 = MS_TIM->CNT + (time_ms << 1); /* 2 kHz */
      MS_TIM->SR = ~TIM_IT_CC1;
      MS_TIM->DIER |= TIM_IT_CC1;
      break;
    case 2:
      ms_callback2 = f;
      MS_TIM->CCR2 = MS_TIM->CNT + (time_ms << 1); /* 2 kHz */
      MS_TIM->SR = ~TIM_IT_CC2;
      MS_TIM->DIER |= TIM_IT_CC2;
      break;
    case 3:
      ms_callback3 = f;
      MS_TIM->CCR3 = MS_TIM->CNT + (time_ms << 1); /* 2 kHz */
      MS_TIM->SR = ~TIM_IT_CC3;
      MS_TIM->DIER |= TIM_IT_CC3;
      break;
    case 4:
      ms_callback4 = f;
      MS_TIM->CCR4 = MS_TIM->CNT + (time_ms << 1); /* 2 kHz */
      MS_TIM->SR = ~TIM_IT_CC4;
      MS_TIM->DIER |= TIM_IT_CC4;
      break;
  }
}

/* Stop time counting. The configured function will not be called.
    timer - timer number, from 1 to 4 */
void TimerStop(int timer) {
  switch (timer) {
    case 1:
      MS_TIM->DIER &= ~TIM_IT_CC1;
      MS_TIM->SR = ~TIM_IT_CC1;
      ms_callback1 = NULL;
      break;
    case 2:
      MS_TIM->DIER &= ~TIM_IT_CC2;
      MS_TIM->SR = ~TIM_IT_CC2;
      ms_callback2 = NULL;
      break;
    case 3:
      MS_TIM->DIER &= ~TIM_IT_CC3;
      MS_TIM->SR = ~TIM_IT_CC3;
      ms_callback3 = NULL;
      break;
    case 4:
      MS_TIM->DIER &= ~TIM_IT_CC4;
      MS_TIM->SR = ~TIM_IT_CC4;
      ms_callback4 = NULL;
      break;
  }
}

static volatile int wait = 0;

static void EndWait(void) {
  wait = 0;
}

/* Make active delay.
    timer   - timer number, from 1 to 4
    time_ms - time to wait for. */
void ActiveWait(int timer, unsigned time_ms) {
  wait = 1;
  TimerStart(timer, EndWait, time_ms);
  while (wait);
}

/* The callback function is allowed to reenable a timer. */
void MS_TIM_IRQHandler(void) {
  uint16_t it_status;
  void (*callback)(void);

  it_status = MS_TIM->SR & MS_TIM->DIER;
  if (it_status & TIM_IT_CC1) {
    callback = ms_callback1;
    TimerStop(1);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC2) {
    callback = ms_callback2;
    TimerStop(2);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC3) {
    callback = ms_callback3;
    TimerStop(3);
    if (callback)
      callback();
  }
  if (it_status & TIM_IT_CC4) {
    callback = ms_callback4;
    TimerStop(4);
    if (callback)
      callback();
  }
}

#endif /* MS_TIM_N */

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
	tim_oc_preload_config( MS_TIM, tim_cc_chn1, false );
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
