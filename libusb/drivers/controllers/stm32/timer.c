#include <stddef.h>
#include <stm32tim.h>
#include <stm32system.h>
#include <usb/drivers/controllers/stm32/timer.h>
#include <usb/core/xcat.h>

#define MS_TIM_N 2
#define US_TIM_N 3

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
void TimerConfigure(unsigned prio, unsigned subprio, unsigned pclk1, unsigned pclk2 ) {
#if 0 
  RCC_ClocksTypeDef       RCC_ClocksStruct;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
  TIM_OCInitTypeDef       TIM_OCInitStruct;
  NVIC_InitTypeDef        NVIC_InitStruct;
  uint32_t                prescaler;

  RCC_APB1PeriphClockCmd(MS_TIM_RCC, ENABLE);

  /* Compute timer prescaler. Timer clock should be 2 kHz. */
  RCC_GetClocksFreq(&RCC_ClocksStruct);
  if (RCC_ClocksStruct.HCLK_Frequency ==
      RCC_ClocksStruct.PCLK1_Frequency) /* APB1 prescaler == 1 */
    prescaler = RCC_ClocksStruct.PCLK1_Frequency / 2000 - 1;
  else
    prescaler = RCC_ClocksStruct.PCLK1_Frequency / 1000 - 1;

  TIM_TimeBaseStructInit(&TIM_TimeBaseStruct);
  TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up; /* default */
  TIM_TimeBaseStruct.TIM_Prescaler = prescaler; /* 2 kHz */
  TIM_TimeBaseInit(MS_TIM, &TIM_TimeBaseStruct);

  TIM_OCStructInit(&TIM_OCInitStruct);
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Timing; /* default */
  TIM_OC1Init(MS_TIM, &TIM_OCInitStruct);
  TIM_OC1PreloadConfig(MS_TIM, TIM_OCPreload_Disable);

  NVIC_InitStruct.NVIC_IRQChannel = MS_TIM_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = subprio;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  TIM_Cmd(MS_TIM, ENABLE);
#endif

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
void FineTimerConfigure(unsigned prio, unsigned subprio) {
#if 0
  RCC_ClocksTypeDef       RCC_ClocksStruct;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
  TIM_OCInitTypeDef       TIM_OCInitStruct;
  NVIC_InitTypeDef        NVIC_InitStruct;
  uint32_t                prescaler;

  RCC_APB1PeriphClockCmd(US_TIM_RCC, ENABLE);

  /* Compute timer prescaler. Timer clock should be 1 MHz. */
  RCC_GetClocksFreq(&RCC_ClocksStruct);
  if (RCC_ClocksStruct.HCLK_Frequency ==
      RCC_ClocksStruct.PCLK1_Frequency) /* APB1 prescaler == 1 */
    prescaler = RCC_ClocksStruct.PCLK1_Frequency / 1000000 - 1;
  else
    prescaler = RCC_ClocksStruct.PCLK1_Frequency / 500000 - 1;

  TIM_TimeBaseStructInit(&TIM_TimeBaseStruct);
  TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up; /* default */
  TIM_TimeBaseStruct.TIM_Prescaler = prescaler; /* 1 MHz */
  TIM_TimeBaseInit(US_TIM, &TIM_TimeBaseStruct);

  TIM_OCStructInit(&TIM_OCInitStruct);
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Timing; /* default */
  TIM_OC1Init(US_TIM, &TIM_OCInitStruct);
  TIM_OC1PreloadConfig(US_TIM, TIM_OCPreload_Disable);

  NVIC_InitStruct.NVIC_IRQChannel = US_TIM_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = subprio;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  TIM_Cmd(US_TIM, ENABLE);
#endif
}

/* Configure microsecond timer to call a function in the future.
    timer   - timer number, from 1 to 4
    f       - function to call when time is elapsed
    time_us - time to wait for, from 1 to 65535 microseconds */
void FineTimerStart(int timer, void (*f)(void), unsigned time_us) {
#if 0
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
#endif
}

/* Stop time counting. The configured function will not be called.
    timer - timer number, from 1 to 4 */
void FineTimerStop(int timer) {
#if 0
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
#endif
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
#warning fix_this
#endif /* US_TIM_N */
