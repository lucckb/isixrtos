#pragma once

#include <isix/prv/list.h>
#include <isix/prv/types.h>
#include <isix/arch/sem_atomic.h>
#include <isix/config.h>
#if CONFIG_ISIX_SEM_EVENT_NOTIFY
#include <stdatomic.h>
struct isix_event;
#endif

//Structure of semaphore
struct isix_semaphore
{
	//Semaphore val
	_isix_port_atomic_sem_t value;
    //Task val waiting for semaphore
    list_entry_t wait_list;
    //Resource type
    bool static_mem;
#if CONFIG_ISIX_SEM_EVENT_NOTIFY
	//Event notified when a token is stored with no waiting task
	struct isix_event* evt;
	atomic_uchar bitno;
#endif
};


//! Fast signal semaphore after wakeup
static inline __attribute__((always_inline))
void _isixp_sem_fast_signal( struct isix_semaphore* sem )
{
	_isix_port_atomic_sem_inc( &sem->value );
}
