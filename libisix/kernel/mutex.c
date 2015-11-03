#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <isix/mutex.h>
#include <string.h>
#include <isix/prv/mutex.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>

#ifdef ISIX_LOGLEVEL_MUTEX
#undef ISIX_CONFIG_LOGLEVEL
#define ISIX_CONFIG_LOGLEVEL ISIX_LOGLEVEL_MUTEX
#endif
#include <isix/prv/printk.h>

// Create mutex
osmtx_t isix_mutex_create( osmtx_t mutex )
{
	const bool static_mem = (mutex!=NULL);
	if( !mutex ) {
		mutex = (osmtx_t)isix_alloc(sizeof(struct isix_mutex));
		if( !mutex ) {
			pr_err("Unable to allocate memory");
			return NULL;
		}
	}
	memset( mutex, 0, sizeof(*mutex) );
	mutex->static_mem = static_mem;
	list_init( &mutex->wait_list );
	return mutex;
}

// Mutex lock
int isix_mutex_lock( osmtx_t mutex )
{
	if( !mutex ) {
		pr_err("No mutex");
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	//! If owner is assigned
	if( mutex->owner ) {
		if( mutex->count < 1 ) {
			isix_bug( "mutex counter is not positive" );
		}
		//If mutex already owned increment count only
		if( mutex->owner == currp )
		{
			++mutex->count;
		}
		//Mutex lock by another thread
		else
		{
			/* Priority inheritance protocol gain thread prio waiting on mtx */
			if( isixp_prio_gt(currp->prio, mutex->owner->prio) ) {
				_isixp_reallocate_priority( mutex->owner, currp->prio );
			}
			_isixp_set_sleep( OSTHR_STATE_WTMTX );
			_isixp_add_to_prio_queue( &mutex->wait_list, currp );
			currp->obj.mtx = mutex;
			isix_yield();
			isix_exit_critical();
			return currp->obj.dmsg;
		}
	}
	// Mutex is not assigned make the owner
	else
	{
		if( mutex->count != 0 ) {
			isix_bug( "Mutex counter is not zero" );
		}
		++mutex->count;
		mutex->owner = currp;
	}
	isix_exit_critical();
	return ISIX_EOK;
}


// Mutex try lock
int isix_mutex_trylock( osmtx_t mutex )
{
	int ret;
	if( !mutex ) {
		pr_err("No mutex");
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	if( mutex->owner )
	{
		if( mutex->count < 1 ) {
			isix_bug("Invalid mtx lock count");
		}
		if( mutex->owner == currp ) {
			++mutex->count;
			ret = ISIX_EOK;
		} else {
			ret = ISIX_EPERM;
		}
	}
	else
	{
		if( mutex->count != 0 ) {
			isix_bug( "Mutex counter is not zero" );
		}
		++mutex->count;
		mutex->owner = currp;
		ret = ISIX_EOK;
	}
	isix_exit_critical();
	return ret;
}

//Mutex unlock
int isix_mutex_unlock( osmtx_t mutex )
{
	if( !mutex ) {
		pr_err("No mutex");
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	if( !mutex->owner ) {
		isix_exit_critical();
		return ISIX_ENOTLOCKED;
	}
	if( mutex->count < 1 ) {
		isix_bug("Mutex not positive");
	}
	if( mutex->owner != currp ) {
		isix_exit_critical();
		return ISIX_EPERM;
	}
	if( !list_isempty(&mutex->wait_list) )
	{
		
	}

}

