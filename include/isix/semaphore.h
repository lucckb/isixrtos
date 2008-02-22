#ifndef __ISIX_SEMAPHORE_H
#define __ISIX_SEMAPHORE_H

#include <isix/config.h>
#include <isix/types.h>

/*--------------------------------------------------------------*/

struct sem_struct;
typedef struct sem_struct sem_t;


/*--------------------------------------------------------------*/
//Create semaphore
sem_t* sem_create_isr(sem_t *sem,int val,s8 interrupt);

#define sem_create(sem,val) sem_create_isr(sem,val,-1)

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

#endif

