#ifndef __ISIX_SEMAPHORE_H
#define __ISIX_SEMAPHORE_H

#include <isix/config.h>
#include <isix/types.h>
#include <isix/list.h>

/*--------------------------------------------------------------*/
//Structure of semaphore
typedef struct sem_struct
{
    //Semaphore val
    int value;
    //Task val waiting for semaphore
    list_entry_t sem_task;
    //Return reason
    s8 sem_ret;
} sem_t;

#include <isix/scheduler.h>
/*--------------------------------------------------------------*/
//Create semaphore
sem_t* sem_create(sem_t *sem,int val);


/*--------------------------------------------------------------*/
//Wait for semaphore P()
int sem_wait(sem_t *sem,unsigned long timeout);

/*--------------------------------------------------------------*/
//Get semaphore from isr
int sem_get_isr(sem_t *sem);

/*--------------------------------------------------------------*/
//Sem signal V()
int __sem_signal(sem_t *sem,bool isr);

/*--------------------------------------------------------------*/
//Definition of sem and sem ISR
#define sem_signal(sem) __sem_signal(sem,false)

#define sem_signal_isr(sem) __sem_signal(sem,true)

/*--------------------------------------------------------------*/
//Sem value of semaphore
int sem_setval(sem_t *sem,int val);

/*--------------------------------------------------------------*/
//Get value of semaphore
int sem_getval(sem_t *sem);

/*--------------------------------------------------------------*/
//Sem destroy
int sem_destroy(sem_t *sem);

/*--------------------------------------------------------------*/
//Semaphore can by destroyed
static inline bool __sem_can_destroy(sem_t *sem)
{
   return list_isempty(&sem->sem_task);
}

/*--------------------------------------------------------------*/
#endif

