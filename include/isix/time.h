#ifndef __ISIX_TIME_H
#define __ISIX_TIME_H

#include <isix/types.h>
#include <isix/semaphore.h>

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


#endif
