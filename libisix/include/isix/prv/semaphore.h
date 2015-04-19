#pragma once

#include <isix/prv/list.h>
#include <isix/prv/types.h>
#include <isix/port_atomic.h>

//Structure of semaphore
struct isix_semaphore
{
    //Resource type
    bool static_mem;
	//Semaphore val
  	_port_atomic_t value; 
    //Task val waiting for semaphore
    list_entry_t wait_list;
};

