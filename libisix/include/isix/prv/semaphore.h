#pragma once

#include <isix/prv/list.h>
#include <isix/port_atomic.h>

/*--------------------------------------------------------------*/
//Structure of semaphore
struct sem_struct
{
	//Semaphore val
  	_port_atomic_t value; 
    //Task val waiting for semaphore
    list_entry_t sem_task;
    //Resource type
    bool static_mem;
};

/*--------------------------------------------------------------*/

//Semaphore can by destroyed
static inline bool _isixp_sem_can_destroy(struct sem_struct *sem)
{
   return list_isempty(&sem->sem_task);
}

/*--------------------------------------------------------------*/
