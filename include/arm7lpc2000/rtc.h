#ifndef __ASM_RTC_H
#define __ASM_RTC_H

#include <isix/config.h>
#include <isix/types.h>
#include <asm/lpc214x.h>
/*--------------------------------------------*/
//Get microsecond from time
static inline u32 get_usec(void)
{
    return T0TC;
}
/*--------------------------------------------*/
//Get current time in timeval struct
void get_tmtime(struct tm *tmtime);

/*--------------------------------------------*/
//Set current time in timeval value
void set_tmtime(const struct tm *tmtime);

/*--------------------------------------------*/
//Set current time in timeval value
void rtc_clock_init(void);

/*--------------------------------------------*/


#endif
