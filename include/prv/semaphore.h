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
    //Return reason
    int sem_ret;
};

/*--------------------------------------------------------------*/

//Semaphore can by destroyed
static inline bool isixp_sem_can_destroy(sem_t *sem)
{
   return list_isempty(&sem->sem_task);
}

/*--------------------------------------------------------------*/

#endif
