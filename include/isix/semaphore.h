#ifndef __ISIX_SEMAPHORE_H
#define __ISIX_SEMAPHORE_H

#include <isix/config.h>
#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/list.h>

/*--------------------------------------------------------------*/
//Structure of semaphore
typedef struct sem_struct
{
    //Semaphore val
    int value;
    //Task val waiting for semaphore
    list_entry_t sem_task;
} sem_t;

/*--------------------------------------------------------------*/
//Create semaphore
sem_t* sem_create(sem_t *sem,int val);


/*--------------------------------------------------------------*/
//Wait for semaphore P()
int sem_wait(sem_t *sem,time_t timeout);

/*--------------------------------------------------------------*/
//Get semaphore from isr
int sem_get_isr(sem_t *sem);

/*--------------------------------------------------------------*/
//Sem signal V()
int sem_signal(sem_t *sem);

/*--------------------------------------------------------------*/
//Signal from interrupt
int sem_signal_isr(sem_t *sem);

/*--------------------------------------------------------------*/
//Sem value of semaphore
int sem_setval(sem_t *sem,int val);

/*--------------------------------------------------------------*/
//Sem destroy
int sem_destroy(sem_t *sem);


#endif

