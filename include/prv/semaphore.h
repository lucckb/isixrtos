#ifndef __PRV_ISIX_SEMAPHORE_H__
#define __PRV_ISIX_SEMAPHORE_H__


#include <prv/list.h>

/*--------------------------------------------------------------*/
//Structure of semaphore
struct sem_struct
{
    //Semaphore val
    int value;
    //Task val waiting for semaphore
    list_entry_t sem_task;
    //Resource type
    bool static_mem;
    //Semaphore limit value
    int limit_value;
};

/*--------------------------------------------------------------*/

//Semaphore can by destroyed
static inline bool isixp_sem_can_destroy(sem_t *sem)
{
   return list_isempty(&sem->sem_task);
}

/*--------------------------------------------------------------*/

#endif
