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
typedef uint32_t time_t;

/*-----------------------------------------------------------*/
//Get current jiffies
uint64_t get_jiffies(void);

/*-----------------------------------------------------------*/
//Wait for n jiffies
#define schedule_timeout(timeout) sem_wait(NULL,timeout)

/*-----------------------------------------------------------*/

#endif
