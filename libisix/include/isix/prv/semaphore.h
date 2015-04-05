#pragma once

#include <isix/prv/list.h>
#include <isix/port_atomic.h>


//Structure of semaphore
struct isix_semaphore
{
	//Semaphore val
  	_port_atomic_t value; 
    //Task val waiting for semaphore
    list_entry_t wait_list;
    //Resource type
    bool static_mem;
};


//Semaphore can by destroyed
static inline bool _isixp_sem_can_destroy(struct isix_semaphore *sem)
{
   return list_isempty(&sem->wait_list);
}

