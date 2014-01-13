#ifndef _USB_TIMER_H
#define _USB_TIMER_H 1

#ifdef __cplusplus
extern "C" {
#endif

void TimerConfigure(unsigned, unsigned, unsigned, unsigned );
void TimerStart(int, void (*)(void), unsigned);
void TimerStop(int);
void ActiveWait(int, unsigned);

void FineTimerConfigure(unsigned, unsigned);
void FineTimerStart(int, void (*)(void), unsigned);
void FineTimerStop(int);

#ifdef __cplusplus
}
#endif

#endif
