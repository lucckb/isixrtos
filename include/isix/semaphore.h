#ifndef __ISIX_SEMAPHORE_H
#define __ISIX_SEMAPHORE_H

#include <isix/config.h>
#include <isix/types.h>
#include <isix/scheduler.h>

/*--------------------------------------------------------------*/
//TODO: Fill structure
//Structure of semaphore
typedef struct sem_struct
{
    //TMP hack
    int tst;
} sem_t;
/*--------------------------------------------------------------*/
//Create semaphore
sem_t* sem_create(sem_t *sem,int val);

/*--------------------------------------------------------------*/
//Wait for semaphore P()
int sem_wait(sem_t *sem,u32 timeout);

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

