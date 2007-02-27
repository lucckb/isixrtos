#ifndef __ISIX_TIME_H
#define __ISIX_TIME_H

#include <isix/types.h>
#include <isix/semaphore.h>

/*-----------------------------------------------------------*/
// Tm structure for time operations
struct tm
{
     unsigned char     tm_sec;         /* sekundy */
     unsigned char     tm_min;         /* minuty */
     unsigned char     tm_hour;        /* godziny */
     unsigned char     tm_mday;        /* dzień miesiąca */
     unsigned char     tm_mon;         /* miesiąc */
     unsigned short    tm_year;        /* rok */
     unsigned char     tm_wday;        /* dzień tygodnia */
     unsigned short    tm_yday;        /* dzień roku */
};

/*-----------------------------------------------------------*/
//Definition of time_t
typedef u32 time_t;

/*-----------------------------------------------------------*/
//Get current jiffies
u64 get_jiffies(void);

/*-----------------------------------------------------------*/
//Wait for n jiffies
#define schedule_timeout(timeout) sem_wait(NULL,timeout)


/*------------------------------------------------------------*/
//Convert tm to time_t
time_t mktime(const struct tm *t);

/*------------------------------------------------------------*/
//Get time in time_t format
time_t time(time_t *t);

/*------------------------------------------------------------*/
#include <asm/rtc.h>

#endif
