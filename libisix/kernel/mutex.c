#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <isix/mutex.h>
#include <string.h>
#include <isix/prv/mutex.h>
#include <isix/prv/scheduler.h>

#ifdef CONFIG_ISIX_LOGLEVEL_MUTEX
#undef CONFIG_ISIX_LOGLEVEL
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_MUTEX
#endif
#include <isix/prv/printk.h>




//! Helper function for transfer mutex ownership
static inline ostask_t transfer_mtx_ownership_to_next_waiting_task( osmtx_t mutex )
{
	mutex->count = 1;
	ostask_t next_tsk = list_first_entry(&mutex->wait_list,inode,struct isix_task);
	mutex->owner = next_tsk;
	list_delete( &next_tsk->inode );
	list_insert_first( &next_tsk->owned_mutexes, &mutex->inode );
	return next_tsk;
}

//! Helper function set ownership
static inline void set_ownership_to_current( osmtx_t mutex )
{
	if( mutex->count != 0 ) {
		isix_bug( "Mutex counter is not zero" );
	}
	++mutex->count;
	mutex->owner = currp;
	list_insert_first( &currp->owned_mutexes, &mutex->inode );
}


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
			if( _isixp_prio_gt(currp->prio, mutex->owner->prio) ) {
				_isixp_reallocate_priority( mutex->owner, currp->prio );
			}
			_isixp_set_sleep( OSTHR_STATE_WTMTX );
			_isixp_add_to_prio_queue( &mutex->wait_list, currp );
			currp->obj.mtx = mutex;
			isix_exit_critical();
			isix_yield();
			return currp->obj.dmsg;
		}
	}
	// Mutex is not assigned make the owner
	else
	{
		set_ownership_to_current( mutex );
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
		set_ownership_to_current( mutex );
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
	if( --mutex->count == 0 )
	{
		//Check and remove fist element from the owning mutexes list
		//It should be the same mutex list passed by argument
		{
			osmtx_t lfirst = NULL;
			list_for_each_entry( &currp->owned_mutexes, lfirst, inode ) {
				if( lfirst == mutex ) break;
			}
			if( !lfirst ) {
				isix_bug("Not in mutex ownership list");
			}
			list_delete( &lfirst->inode );
		}
		if( !list_isempty(&mutex->wait_list) )
		{
			//Recalculate the new mutex task priority
			osmtx_t mtx;
			osprio_t newprio = currp->real_prio;
			list_for_each_entry( &currp->owned_mutexes, mtx, inode )
			{
				if( !list_isempty(&mtx->wait_list) ) {
					ostask_t t = list_first_entry(&mtx->wait_list,inode,struct isix_task);
					if( _isixp_prio_gt(t->prio,newprio) ) {
						newprio = t->prio;
					}
				}
			}
			_isixp_reallocate_priority( currp, newprio );
			_isixp_wakeup_task( transfer_mtx_ownership_to_next_waiting_task(mutex), ISIX_EOK );
			return ISIX_EOK;
		}
		else {
			mutex->owner = NULL;
		}
	}
	isix_exit_critical();
	return ISIX_EOK;
}


//! Unlock all waiting threads
void _isixp_mutex_unlock_all_in_task( ostask_t utask )
{
	ostask_t wkup_task = NULL;
	isix_enter_critical();
	if( !list_isempty( &utask->owned_mutexes) )
	{
		osmtx_t mtx, tmp;
		list_for_each_entry_safe( &utask->owned_mutexes, mtx, tmp, inode )
		{
			if( !list_isempty( &mtx->wait_list ) )
			{
				ostask_t t = transfer_mtx_ownership_to_next_waiting_task(mtx);
				_isixp_wakeup_task_l( t , ISIX_EOK );
				//NOTE: Wait list is prioritized so the first has highest prio
				if( !wkup_task ) wkup_task = t;
			}
			else
			{
				list_delete( &mtx->inode );
				mtx->count = 0;
				mtx->owner = NULL;
			}
		}
	}
	if( wkup_task ) {
		_isixp_do_reschedule( wkup_task );
	} else {
		isix_exit_critical();
	}
}

//! Unlock all waiting threads
void isix_mutex_unlock_all(void) {
	_isixp_mutex_unlock_all_in_task( currp  );
}

/** Destroy the recursive mutex
 * @param[in] mutex Recursive mutex object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_mutex_destroy( osmtx_t mutex )
{
	if( !mutex ) {
		pr_err( "Invalid mutex identifier");
		return ISIX_EINVARG;
	}
	ostask_t tsk, tmp;
	ostask_t wkup_task = NULL;
	isix_enter_critical();
	if( mutex->owner )
		list_delete( &mutex->inode );

	list_for_each_entry_safe( &mutex->wait_list, tsk, tmp, inode )
	{
		_isixp_reallocate_priority( tsk, tsk->real_prio );
		list_delete( &tsk->inode );
		//NOTE: Wait list is prioritized so the first has highest prio
		if( !wkup_task ) wkup_task = tsk;
		_isixp_wakeup_task_l( tsk, ISIX_EDESTROY );
	}
	if( wkup_task ) {
		_isixp_do_reschedule( wkup_task );
	} else {
		isix_exit_critical();
	}
	if( !mutex->static_mem ) isix_free( mutex );
	return ISIX_EOK;
}

//! Get first Mutex owner and release it
osmtx_t _isixp_get_top_currt_mutex( void )
{
	if( list_isempty(&currp->owned_mutexes) ) {
		return NULL;
	} else {
		return list_first_entry(&currp->owned_mutexes,inode,struct isix_mutex);
	}
}
