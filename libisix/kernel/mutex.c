#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <isix/mutex.h>
#include <string.h>
#include <isix/prv/mutex.h>
#include <isix/prv/scheduler.h>
#include <isix/assert.h>

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
	// Detach from the previous owner list when it was not done by the caller
	if( list_is_elem_assigned( &mutex->inode ) ) {
		list_delete( &mutex->inode );
	}
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


//! Priority of the task including the highest waiter on any mutex it owns
static osprio_t calc_inherited_priority( ostask_t owner )
{
	osmtx_t mtx;
	osprio_t newprio = owner->real_prio;
	list_for_each_entry( &owner->owned_mutexes, mtx, inode )
	{
		if( !list_isempty(&mtx->wait_list) ) {
			const ostask_t t = list_first_entry(&mtx->wait_list,inode,struct isix_task);
			if( _isixp_prio_gt(t->prio,newprio) ) {
				newprio = t->prio;
			}
		}
	}
	return newprio;
}

//! Recalculate owner priority after a waiter left the wait list
void _isixp_mutex_waiter_removed( osmtx_t mutex )
{
	const ostask_t owner = mutex->owner;
	if( !owner ) {
		return;
	}
	const osprio_t newprio = calc_inherited_priority( owner );
	if( newprio != owner->prio ) {
		_isixp_reallocate_priority( owner, newprio );
	}
}

// Create mutex
osmtx_t isix_mutex_create( osmtx_t mutex )
{
    isix_assert_isr();
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
    isix_assert_isr();
	if( !mutex ) {
		pr_err("No mutex");
		return ISIX_EINVARG;
	}
	for(;;)
	{
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
				currp->wait_aborted = false;
				_isixp_set_sleep( OSTHR_STATE_WTMTX );
				_isixp_add_to_prio_queue( &mutex->wait_list, currp );
				currp->obj.mtx = mutex;
				isix_exit_critical();
				isix_yield();
				// Resumed after suspend without owning the mutex, wait again
				if( currp->wait_aborted ) {
					continue;
				}
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
}

// Mutex try lock
int isix_mutex_trylock( osmtx_t mutex )
{
	int ret;
    isix_assert_isr();
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
    isix_assert_isr();
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
			osmtx_t lfirst;
			bool found = false;
			list_for_each_entry( &currp->owned_mutexes, lfirst, inode ) {
				if( lfirst == mutex ) {
					found = true;
					break;
				}
			}
			if( !found ) {
				isix_bug("Not in mutex ownership list");
			}
			list_delete( &mutex->inode );
		}
		if( !list_isempty(&mutex->wait_list) )
		{
			const osprio_t newprio = calc_inherited_priority( currp );
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
	if( utask == currp ) {
		// Drop the priority inherited through the released mutexes
		const osprio_t newprio = calc_inherited_priority( utask );
		if( newprio != utask->prio ) {
			_isixp_reallocate_priority( utask, newprio );
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
    isix_assert_isr();
	_isixp_mutex_unlock_all_in_task( currp );
}

/** Destroy the recursive mutex
 * @param[in] mutex Recursive mutex object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_mutex_destroy( osmtx_t mutex )
{
    isix_assert_isr();
	if( !mutex ) {
		pr_err( "Invalid mutex identifier");
		return ISIX_EINVARG;
	}
	ostask_t tsk, tmp;
	ostask_t wkup_task = NULL;
	isix_enter_critical();
	const ostask_t owner = mutex->owner;
	if( owner ) {
		list_delete( &mutex->inode );
		mutex->owner = NULL;
		mutex->count = 0;
	}
	list_for_each_entry_safe( &mutex->wait_list, tsk, tmp, inode )
	{
		list_delete( &tsk->inode );
		// Keep the priority inherited through other mutexes owned by the waiter
		tsk->prio = calc_inherited_priority( tsk );
		//NOTE: Wait list is prioritized so the first has highest prio
		if( !wkup_task ) wkup_task = tsk;
		_isixp_wakeup_task_l( tsk, ISIX_EDESTROY );
	}
	if( owner ) {
		// Drop the priority inherited from the waiters of this mutex
		const osprio_t newprio = calc_inherited_priority( owner );
		if( newprio != owner->prio ) {
			_isixp_reallocate_priority( owner, newprio );
		}
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
