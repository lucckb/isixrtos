#ifndef __ISIX_TIME_H
#define __ISIX_TIME_H

#include <isix/types.h>
#include <isix/semaphore.h>
#include <asm/rtc.h>
/*-----------------------------------------*/
//Definition of time_t
typedef u32 time_t;

/*-----------------------------------------*/
//Get current jiffies
u64 get_jiffies(void);

/*-----------------------------------------*/
//Wait for n jiffies
#define schedule_timeout(timeout) sem_wait(NULL,timeout);

/*-----------------------------------------*/
// Tm structure for time operations
struct tm
{
     u8     tm_sec;         /* sekundy */
     u8     tm_min;         /* minuty */
     u8     tm_hour;        /* godziny */
     u8     tm_mday;        /* dzień miesiąca */
     u8     tm_mon;         /* miesiąc */
     u16    tm_year;        /* rok */
     u16    tm_wday;        /* dzień tygodnia */
     u16    tm_yday;        /* dzień roku */
     bool   tm_isdst;       /* sezonowa zmiana czasu */
};

#endif
