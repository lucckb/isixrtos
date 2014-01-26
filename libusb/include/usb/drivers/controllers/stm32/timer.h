#ifndef _USBLIB_TIMER_H
#define _USBLIB_TIMER_H 1

#ifdef __cplusplus
extern "C" {
#endif

void TimerConfigure(unsigned, unsigned, unsigned);
void TimerStart(int, void (*)(void), unsigned);
void TimerStop(int);
void ActiveWait(int, unsigned);

void FineTimerConfigure(unsigned, unsigned, unsigned);
void FineTimerStart(int, void (*)(void), unsigned);
void FineTimerStop(int);

void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif
