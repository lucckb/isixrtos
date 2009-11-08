#ifndef __ISIX_SEMAPHORE_H
#define __ISIX_SEMAPHORE_H

#include <isix/config.h>
#include <isix/types.h>

/*--------------------------------------------------------------*/

struct sem_struct;
typedef struct sem_struct sem_t;


/*--------------------------------------------------------------*/
//Create semaphore
sem_t* isix_sem_create_isr(sem_t *sem,int val,int interrupt);

#define isix_sem_create(sem,val) isix_sem_create_isr(sem,val,-1)

/*--------------------------------------------------------------*/
//Wait for semaphore P()
int isix_sem_wait(sem_t *sem, tick_t timeout);

/*--------------------------------------------------------------*/
//Get semaphore from isr
int isix_sem_get_isr(sem_t *sem);

/*--------------------------------------------------------------*/
//Sem signal V()
int isixp_sem_signal(sem_t *sem, bool isr);

/*--------------------------------------------------------------*/
//Definition of sem and sem ISR
#define isix_sem_signal(sem) isixp_sem_signal(sem,false)

#define isix_sem_signal_isr(sem) isixp_sem_signal(sem,true)

/*--------------------------------------------------------------*/
//Sem value of semaphore
int isix_sem_setval(sem_t *sem,int val);

/*--------------------------------------------------------------*/
//Get value of semaphore
int isix_sem_getval(sem_t *sem);

/*--------------------------------------------------------------*/
//Sem destroy
int isix_sem_destroy(sem_t *sem);

/*--------------------------------------------------------------*/

//Wait for n jiffies
#define isix_wait(timeout) isix_sem_wait(NULL,timeout)

/*-----------------------------------------------------------*/

#endif

