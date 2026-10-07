#include <isix/config.h>
#include <isix/types.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <isix/prv/list.h>
#include <isix/prv/softtimers.h>
#include <isix/prv/semaphore.h>
#include <isix/prv/mutex.h>
#include <isix/prv/osstats.h>
#include <isix/prv/condvar.h>
#include <isix/prv/mmalloc.h>
#include <isix/prv/scheduler.h>
#include <isix/prv/test_hooks.h>
#include <isix/arch/memprot.h>
#include <isix/arch/cpu.h>
#include <isix/arch/core.h>
#include <isix/arch/ostimer.h>
#include <isix/assert.h>
#include <stdatomic.h>
#include <stdint.h>

#ifdef CONFIG_ISIX_LOGLEVEL_SCHEDULER
#undef CONFIG_ISIX_LOGLEVEL
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_SCHEDULER
#endif
#include <isix/prv/printk.h>


//Current task simple def
static ISIX_TASK_FUNC(idle_task,p);
static void add_ready_list( ostask_t task );
static void cleanup_tasks(void);
static void advance_jiffies(ostick_t ticks);
#if CONFIG_ISIX_TICKLESS
static void enter_tickless_idle(void);
static void catch_up_missed_ticks(ostick_t elapsed);
static ostick_t calculate_next_timeout(void);
static bool can_enter_tickless_sleep(void);
#endif

// Task reschedule lock for spinlock
static struct isix_system csys;

#if CONFIG_ISIX_TEST_HOOKS
void (*volatile _isixp_test_hook)( enum isix_test_point point, void* arg ) = NULL;

int _isixp_test_critical_count(void)
{
	return atomic_load( &csys.critical_count );
}
#endif
//Current task pointer
volatile bool _isix_scheduler_running;
/* Referenced from asm in arch scheduler; keep when linking with LTO. */
ostask_t volatile _isix_current_task __attribute__((used));

#if CONFIG_ISIX_TICKLESS && defined(CONFIG_ISIX_TEST)
static volatile bool tickless_test_wfi;
#endif

//! Ununsed systick handler
static void unused_func(void ) {}
void isix_systime_handler(void) __attribute__ ((weak, alias("unused_func")));

//! Kernel panic callback function definition
void __attribute__((weak))
isix_kernel_panic_callback( const char* file, int line, const char *msg )
{
	(void)file; (void)line; (void)msg;
}

#if CONFIG_ISIX_TICKLESS
void __attribute__((weak, noinline, externally_visible))
isix_pre_sleep_hook(ostick_t expected_sleep_ticks)
{
	(void)expected_sleep_ticks;
}

void __attribute__((weak, noinline, externally_visible))
isix_post_sleep_hook(ostick_t actual_sleep_ticks)
{
	(void)actual_sleep_ticks;
}
#endif

//Get currrent jiffies
ostick_t isix_get_jiffies(void)
{
    return atomic_load(&csys.jiffies);
}

#ifdef CONFIG_ISIX_TEST
void _isixp_test_set_jiffies(ostick_t v)
{
	atomic_store(&csys.jiffies, v);
}

void _isixp_test_advance_jiffies(ostick_t dt)
{
	isix_enter_critical();
	advance_jiffies(dt);
	isix_exit_critical();
}
#endif

//Get maxium available priority
osprio_t isix_get_min_priority(void)
{
	return CONFIG_ISIX_NUMBER_OF_PRIORITIES-1;
}

/** Temporary lock task reschedule */
void _isixp_lock_scheduler(void)
{
	_isix_port_atomic_sem_inc( &csys.sched_lock );
}


/** Temporary unlock task reschedule */
void _isixp_unlock_scheduler(void)
{
	const int lock_count = _isix_port_atomic_sem_dec( &csys.sched_lock );
	if( lock_count < 0 ) {
		isix_bug("Scheduler unlock without lock");
	}
	if( lock_count == 0 ) {
		isix_enter_critical();
		const int skipped = atomic_exchange(&csys.jiffies_skipped, 0);
		if( skipped > 0 ) {
			advance_jiffies((ostick_t)skipped);
		}
		if( atomic_load(&csys.yield_pending) ) {
			_isix_port_yield();
			atomic_store(&csys.yield_pending, false );
		}
		isix_exit_critical();
	}
}

#if CONFIG_ISIX_SHUTDOWN_API
/**
 * Shutdown scheduler and return to main
 * @note It can be called only  a once just before
 * the system shutdown for battery power save
 */
void isix_shutdown_scheduler(void)
{
	//Dropout the task prot region
    isix_assert_isr();
	schrun = false;
	_isix_port_memory_protection_reset_efence();
	_isix_port_yield();
}

/** Function called at end of isix execution only
 * when shutdown API is enabled
 */
void _isixp_finalize(void)
{
	_isixp_vtimers_finalize();
	cleanup_tasks();
}
#endif /* CONFIG_ISIX_SHUTDOWN_API  */

//Lock scheduler
void isix_enter_critical(void)
{
	//Mask must be set before the count is visible, else PendSV can switch with count > 0
	_isix_port_set_interrupt_mask();
	int res = atomic_fetch_add( &csys.critical_count, 1 );
	if( res < 0 ) {
		isix_bug("Invalid lock count");
	}
	_isix_port_flush_memory();
}

//Unlock scheduler
void isix_exit_critical(void)
{
	int res = atomic_fetch_sub( &csys.critical_count, 1 );
	if( res <= 0 ) {
		isix_bug("Invalid lock count");
	}
	if( res == 1 ) {
		_isix_port_clear_interrupt_mask();
	}
	_isix_port_flush_memory();
}


/* Number of priorites assigned when OS start */
void isix_init(unsigned long core_freq)
{
	//Schedule lock count
    isix_assert_isr();
	_isix_port_memory_protection_set_default_map();
	//The lock is nestable because the heap allocator uses it
	_isix_port_atomic_sem_init( &csys.sched_lock, 0, sys_atomic_unlimited_value );
	atomic_init( &csys.critical_count, 0 );
	//Copy priority
	//Init heap
	_isixp_alloc_init();
	//Initialize ready task list
    list_init(&csys.ready_list);
    //Initialize waiting list
    list_init(&csys.wait_lists[0]);
    list_init(&csys.wait_lists[1]);
    //Initialize overflow waiting list
    csys.p_wait_list = &csys.wait_lists[0];
    csys.pov_wait_list = &csys.wait_lists[1];
    //Initialize dead task
    list_init(&csys.zombie_list);
    //Initialize free prio elem list
    list_init(&csys.free_prio_elem);
    //This memory never will be freed
    task_ready_t *prio = isix_alloc(
		sizeof(task_ready_t)*(CONFIG_ISIX_NUMBER_OF_PRIORITIES+1)
	);
	if( !prio ) {
		isix_bug("Insufficient memory alloc priority list");
	}
    for(int i=0; i<=CONFIG_ISIX_NUMBER_OF_PRIORITIES; ++i) {
    	list_insert_end(&csys.free_prio_elem,&prio[i].inode);
    }
    //Lower priority is the idle task
	if( !isix_task_create( idle_task, NULL,
			ISIX_PORT_SCHED_MIN_STACK_DEPTH,
			CONFIG_ISIX_NUMBER_OF_PRIORITIES,0 )
	  )
	{
		isix_bug("Insufficient memory idle task");
	}
	_isix_port_conf_hardware( core_freq );
}

//Isix bug report when printk is defined
void isix_kernel_panic( const char *file, int line, const char *msg )
{
    //Go to critical sections forever
	isix_enter_critical();
#if CONFIG_ISIX_LOGLEVEL!=ISIXLOG_OFF
	pr_crit("OOPS-PANIC: Please reset board %s:%i [%s]", file, line, msg );
    task_ready_t *i;
    ostask_t j;
    pr_crit("Ready tasks");
    list_for_each_entry(&csys.ready_list,i,inode)
    {
         pr_crit("\t* List inode %p prio %i",i, i->prio );
         list_for_each_entry(&i->task_list,j,inode)
         {
              pr_crit("\t\t-> task %p prio %i state %i",j,j->prio,j->state);
         }
    }
    pr_crit("Waiting tasks");
    list_for_each_entry(csys.p_wait_list,j,inode_time)
    {
        pr_crit("\t->Task: %p prio: %i state %i jiffies %i",
				j, j->prio, j->state, j->jiffies );
    }
#endif
    isix_kernel_panic_callback( file, line, msg );
    while(1);
}

#if 0
/** Temporary debug */
void print_task_list()
{
	isix_enter_critical();
    task_ready_t *i;
    ostask_t j;
	tiny_printf("curr %p:%i jiff %u Ready tasks\r\n", currp, currp->prio, csys.jiffies );
    list_for_each_entry(&csys.ready_list,i,inode)
    {
         tiny_printf("\t* List inode %p prio %i\r\n",i, i->prio );
         list_for_each_entry(&i->task_list,j,inode)
         {
              tiny_printf("\t\t-> task %p prio %i state %i\r\n",j,j->prio,j->state);
         }
    }
    tiny_printf("Waiting tasks\r\n");
    list_for_each_entry(csys.p_wait_list,j,inode_time)
    {
        tiny_printf("\t->Task: %p prio: %i state %i jiffies %i\r\n",
			j, j->prio, j->state, j->jiffies );
    }
	tiny_printf("Zombie tasks\r\n");
	list_for_each_entry(&csys.zombie_list,j,inode)
	{
        tiny_printf("\t->Task: %p prio: %i state %i jiffies %i\r\n",
			j, j->prio, j->state, j->jiffies );
	}
	isix_exit_critical();
}
#endif

//Scheduler is called in switch context
/**
 * NOTE: The process not require isix_enter_critical because
 * it is protected itself by pend svc vector lock 
 */
void _isixp_schedule(void)
{
	if(  atomic_load( &csys.critical_count ) < 0 ) {
		isix_bug("Critical count fail" );
	}
	if( _isix_port_atomic_sem_read_val(&csys.sched_lock) ) {
		atomic_store( &csys.yield_pending, true );
		return;
	}
    //Remove executed task and add at end
	if( currp->state == OSTHR_STATE_RUNNING ) {
		if( !currp->inode.next || !currp->inode.prev ) {
			isix_bug("RUNNING task inode is not linked");
		}
		list_delete(&currp->inode);
		list_insert_end( &currp->prio_elem->task_list, &currp->inode );
		currp->state = OSTHR_STATE_READY;
	}
    //Get first ready prio
    task_ready_t * curr_prio
		= list_first_entry( &csys.ready_list, inode, task_ready_t );
    pr_debug( "tsk prio %i priolist %p", curr_prio->prio, curr_prio );
    pr_debug( "Scheduler: prev task %p",currp );
    currp = list_first_entry( &curr_prio->task_list, inode,struct isix_task );
	if( currp->state != OSTHR_STATE_READY ) {
		pr_crit("Currp %p state %i", currp, currp->state );
		isix_bug( "Not in READY state. Mem corrupted?" );
	}
	currp->state = OSTHR_STATE_RUNNING;
	//Handle fence stuff
#	if CONFIG_ISIX_MEMORY_PROTECTION_MODEL > 0
	_isix_port_memory_protection_reset_efence( );
	_isix_port_memory_protection_set_efence( currp->fence_estack );
#	endif
	//Update statistics
	_isixp_schedule_update_statistics(
		atomic_load(&csys.jiffies), _isixp_is_idle_prio(currp->prio)
	);
	//Handle local thread errno
	if(currp->impure_data )
	{
		_REENT = currp->impure_data;
	}
	else
	{
		if( _REENT != _GLOBAL_REENT )
			_REENT = _GLOBAL_REENT;
	}
    if(currp->prio != curr_prio->prio)
    {
		pr_crit("tsk: %p %i != %i ", currp, currp->prio , curr_prio->prio );
		isix_bug("Task priority doesn't match to element priority");
    }
}

//Wake the tasks and timers with the deadline reached
static void expire_waiting_tasks( ostick_t jiffies )
{
	ostask_t task_c;
	while( !list_isempty(csys.p_wait_list) &&
		jiffies >=
		(task_c=list_first_entry(csys.p_wait_list,inode_time,struct isix_task))->jiffies
	)
	{
		pr_debug("schedtime: task %p jiffies %i task_time %i", task_c,jiffies,task_c->jiffies);
		list_delete(&task_c->inode_time);
		if( task_c->state == OSTHR_STATE_WTSEM ) {
			/*
			if( _isixp_remove_from_prio_queue(&task_c->obj.sem->wait_list)!=task_c ) {
				isix_bug("Mismatch semaphore task");
			}*/
			//Much faster but less safe
			list_delete( &task_c->inode );
			_isixp_sem_fast_signal( task_c->obj.sem );
			task_c->obj.dmsg = ISIX_ETIMEOUT;
		} else if( task_c->state == OSTHR_STATE_WTEVT ) {
			list_delete( &task_c->inode );
			task_c->obj.dmsg = ISIX_ETIMEOUT;
		} else if( task_c->state == OSTHR_STATE_WTCOND ) {
			list_delete( &task_c->inode );
			task_c->obj.dmsg = ISIX_ETIMEOUT;
		}
		add_ready_list( task_c );
	}
}

//Advance the system time by the number of ticks, the critical section must be held
static void advance_jiffies( ostick_t ticks )
{
	while( ticks > 0U )
	{
		const ostick_t now = atomic_load(&csys.jiffies);
		// Number of ticks after which the counter becomes zero
		const uint64_t to_wrap = (uint64_t)ISIX_TIME_MAX_TICK - now + 1ULL;
		if( (uint64_t)ticks < to_wrap )
		{
			atomic_fetch_add(&csys.jiffies, ticks);
			if( schrun ) {
				expire_waiting_tasks( now + ticks );
			}
			return;
		}
		if( to_wrap > 1U )
		{
			atomic_fetch_add(&csys.jiffies, (ostick_t)(to_wrap - 1U));
			if( schrun ) {
				expire_waiting_tasks( ISIX_TIME_MAX_TICK );
			}
		}
		atomic_fetch_add(&csys.jiffies, 1U);
		if( schrun )
		{
			// The deadlines after the wrap are on the overflow list
			list_entry_t *tmp = csys.p_wait_list;
			csys.p_wait_list = csys.pov_wait_list;
			csys.pov_wait_list = tmp;
			expire_waiting_tasks( 0U );
		}
		ticks -= (ostick_t)to_wrap;
	}
}

#if CONFIG_ISIX_TICKLESS
static void catch_up_missed_ticks(ostick_t elapsed)
{
	if( elapsed == 0U || !schrun ) {
		return;
	}
	if( _isix_port_atomic_sem_read_val(&csys.sched_lock) ) {
		atomic_fetch_add(&csys.jiffies_skipped, (int)elapsed);
		return;
	}
	advance_jiffies(elapsed);
}

static bool can_enter_tickless_sleep(void)
{
	if( _isix_port_atomic_sem_read_val(&csys.sched_lock) ) {
		return false;
	}
	if( !list_isempty(&csys.ready_list) ) {
		task_ready_t *prio = list_first_entry(&csys.ready_list, inode, task_ready_t);
		if( prio->prio < CONFIG_ISIX_NUMBER_OF_PRIORITIES ) {
			return false;
		}
	}
	if( atomic_load(&csys.yield_pending) ) {
		return false;
	}
	if( atomic_load(&csys.jiffies_skipped) > 0 ) {
		return false;
	}
	// The idle task has to reclaim the finished tasks
	return csys.number_of_task_deleted == 0U;
}

static ostick_t calculate_next_timeout(void)
{
	ostick_t current_jiffies = atomic_load(&csys.jiffies);
	uint64_t tw64 = (uint64_t)ISIX_TIME_MAX_TICK - (uint64_t)current_jiffies + 1ULL;
	if( tw64 > (uint64_t)ISIX_TIME_MAX_TICK ) {
		tw64 = (uint64_t)ISIX_TIME_MAX_TICK;
	}
	ostick_t ticks_to_wraparound = (ostick_t)tw64;
	ostick_t ticks_until_timeout = ISIX_TIME_MAX_TICK;

	if( !list_isempty(csys.p_wait_list) ) {
		ostask_t first = list_first_entry(csys.p_wait_list, inode_time, struct isix_task);
		if( first->jiffies >= current_jiffies ) {
			ticks_until_timeout = first->jiffies - current_jiffies;
		}
	}
	ostick_t vtd = _isixp_vtimers_next_timeout_delta(current_jiffies);
	if( vtd < ticks_until_timeout ) {
		ticks_until_timeout = vtd;
	}
	// Deadlines after the wrap are not known here, wake up at the wrap
	if( ticks_to_wraparound < ticks_until_timeout ) {
		ticks_until_timeout = ticks_to_wraparound - 1U;
	}
	if( ticks_until_timeout == 0U ) {
		ticks_until_timeout = 1U;
	}
	return ticks_until_timeout;
}

static void enter_tickless_idle(void)
{
	const ostick_t jiffies_before = atomic_load(&csys.jiffies);
	isix_enter_critical();
	if( _isix_port_systimer_is_sleeping() || !can_enter_tickless_sleep() ) {
		isix_exit_critical();
		_isix_port_idle_cpu();
		return;
	}
	const ostick_t next = calculate_next_timeout();
	const ostick_t timeout = (next < (ostick_t)CONFIG_ISIX_TICKLESS_MIN_SLEEP_TICKS) ?
		1U : _isix_port_systimer_set_timeout(next);
	if( timeout < 2U ) {
		isix_exit_critical();
		_isix_port_idle_cpu();
		return;
	}
	isix_pre_sleep_hook(timeout);
	// Tick interrupts and interrupts that wake nothing do not end the sleep
	do {
		isix_exit_critical();
#ifdef CONFIG_ISIX_TEST
		if( tickless_test_wfi ) {
			asm volatile("wfi\t\n");
		} else
#endif
		{
			_isix_port_idle_cpu();
		}
		isix_enter_critical();
	} while( _isix_port_systimer_is_sleeping() && can_enter_tickless_sleep() );
	catch_up_missed_ticks(_isix_port_systimer_resync());
	isix_post_sleep_hook(atomic_load(&csys.jiffies) - jiffies_before);
	isix_exit_critical();
}

#ifdef CONFIG_ISIX_TEST
void _isixp_test_tickless_force_wfi( bool enable )
{
	tickless_test_wfi = enable;
}
#endif
#endif /* CONFIG_ISIX_TICKLESS */


//Account the ticks reported by the system timer
void _isixp_systimer_announce( ostick_t ticks )
{
	//Call isix system time handler if used
	isix_systime_handler();
	if( _isix_port_atomic_sem_read_val( &csys.sched_lock ) ) {
		atomic_fetch_add( &csys.jiffies_skipped, (int)ticks );
	} else {
		advance_jiffies( ticks );
	}
}

//Try get task ready from free list if is not exist allocate memory
static task_ready_t *alloc_task_ready_t(void)
{
   task_ready_t *prio = NULL;
   if( list_isempty(&csys.free_prio_elem) )
   {
       isix_bug("Priority list not available");
   }
   else
   {
        //Get element from list
        prio = list_first_entry( &csys.free_prio_elem,inode, task_ready_t );
        list_delete( &prio->inode );
        prio->prio = 0;
        pr_debug("alloc_task_ready_t: get from list node %p",prio);
   }
   return prio;
}

//Try get task ready from free list if is not exist allocate memory
static inline void free_task_ready_t(task_ready_t *prio)
{
    list_insert_end(&csys.free_prio_elem,&prio->inode);
}


//Add assigned task to ready list
static void add_ready_list( ostask_t task )
{
    if( task->prio > CONFIG_ISIX_NUMBER_OF_PRIORITIES ) {
		isix_bug("Invalid task priority");
	}
	pr_debug("add: trying to add %p prio %i", task, task->prio );
	if( task->state == OSTHR_STATE_READY ||
		task->state == OSTHR_STATE_ZOMBIE )
	{
		pr_debug(" task_id %p state %i", task, task->state );
		isix_bug( "add: in READY or ZOMBIE state" );
	}
	task->state = OSTHR_STATE_READY;		//Set task to ready state
    //Find task equal entry
    task_ready_t *prio_i;
    list_for_each_entry(&csys.ready_list,prio_i,inode)
    {
        //If task equal entry is found add this task to end list
        if(prio_i->prio==task->prio)
        {
            pr_debug("ardy:  prio %i equal node %p",prio_i->prio,prio_i);
            //Set pointer to priority struct
            task->prio_elem = prio_i;
            //Add task at end of ready list
            list_insert_end(&prio_i->task_list,&task->inode);
            return ;
        }
        else if( _isixp_prio_gt(task->prio,prio_i->prio) )
        {
           pr_debug("wkup: Insert prio %i node %p",prio_i->prio,prio_i);
           break;
        }
    }
    //Priority not found allocate priority node
    task_ready_t *prio_n = alloc_task_ready_t();
    prio_n->prio = task->prio;			//Assign priority
    task->prio_elem = prio_n;			//Set pointer to priority struct
    list_init(&prio_n->task_list);		//Initialize and add at end of list
    list_insert_end( &prio_n->task_list, &task->inode );
    list_insert_before( &prio_i->inode, &prio_n->inode );
	pr_debug("ardy: task state %i", task->state );
    pr_debug("ardy: Add new node %p with prio %i",prio_n,prio_n->prio);
}

//! Delete task from ready list
static void delete_from_ready_list( ostask_t task )
{
	//Scheduler lock
	list_delete(&task->inode);
	//Check for task on priority structure
	if( list_isempty(&task->prio_elem->task_list) )
	{
		//Task list is empty remove element
		list_delete( &task->prio_elem->inode );
		free_task_ready_t( task->prio_elem );
	}
}

//Move selected task to waiting list
static void add_task_to_waiting_list(ostask_t task, ostick_t timeout)
{
    //Scheduler lock
	const ostick_t jiffies=atomic_load(&csys.jiffies);
    task->jiffies = jiffies + timeout;
    if(task->jiffies < jiffies)
    {
    	//Insert on overflow waiting list in time order
    	ostask_t waitl;
    	list_for_each_entry(csys.pov_wait_list,waitl,inode_time)
    	{
    	   if( task->jiffies < waitl->jiffies ) break;
    	}
    	pr_debug("MoveTaskToWaiting: OVERFLOW insert in time list at %p",&waitl->inode);
    	list_insert_before(&waitl->inode_time,&task->inode_time);
    }
    else
    {
    	//Insert on waiting list in time order no overflow
    	ostask_t waitl;
    	list_for_each_entry(csys.p_wait_list,waitl,inode_time)
    	{
    	    if(task->jiffies<waitl->jiffies) break;
    	}
    	pr_debug("MoveTaskToWaiting: NO overflow insert in time list at %p",&waitl->inode);
    	list_insert_before(&waitl->inode_time,&task->inode_time);
    }
}

//Add task to the list according to current priority calculation
void _isixp_add_to_prio_queue( list_entry_t *list, ostask_t task )
{
    //Insert on waiting list in time order
    ostask_t item;
    list_for_each_entry( list, item, inode )
    {
    	if ( _isixp_prio_gt(task->prio,item->prio) ) break;
    }
    pr_debug("prioqueue: insert in time list at %p", task );
    list_insert_before( &item->inode, &task->inode );
}

//! Remove task from prio queue
ostask_t _isixp_remove_from_prio_queue( list_entry_t* list )
{
	if( list_isempty( list ) ) {
		return NULL;
	}
	ostask_t task = list_first_entry( list, inode, struct isix_task );
	list_delete( &task->inode );
	return task;
}


//Dead task are clean by this procedure called from idle task
//One idle call clean one dead tasks
static void cleanup_tasks(void)
{
    if( csys.number_of_task_deleted > 0 )
    {
		ostask_t task_del = NULL;
		ostask_t to_free = NULL;
		void* stack = NULL;
		void* reent = NULL;
        isix_enter_critical();
        if(!list_isempty(&csys.zombie_list))
        {
			task_del = list_first_entry(&csys.zombie_list,inode,struct isix_task);
            list_delete(&task_del->inode);
			pr_info( "Task to delete: %p(SP %p) PRIO: %i",
						task_del,task_del->init_stack,task_del->prio );
			stack = task_del->init_stack;
			task_del->init_stack = NULL;
			reent = task_del->impure_data;
			task_del->impure_data = NULL;
			if( task_del->refcnt == 0 ) to_free = task_del;
			task_del->state = OSTHR_STATE_EXITED;
			csys.number_of_task_deleted--;
        }
        isix_exit_critical();
		if( task_del ) {
			// After leaving the critical section isix_task_unref may free the TCB
			ISIX_TEST_POINT( isix_tp_task_cleanup, task_del );
			isix_free( stack );
			if( reent ) isix_free( reent );
			if( to_free ) isix_free( to_free );
		}
    }
}

//Idle task function do nothing and lower priority
ISIX_TASK_FUNC(idle_task,p)
{
	(void)p;
	while(1)
    {
        //Cleanup free tasks
        cleanup_tasks();
#if CONFIG_ISIX_TICKLESS
        enter_tickless_idle();
#else
        //Call port specific idle
        _isix_port_idle_cpu();
#endif
    }
}

/* This function start scheduler after main function */
#if !(CONFIG_ISIX_SHUTDOWN_API)
void isix_start_scheduler(void) __attribute__((noreturn));
#endif
void isix_start_scheduler(void)
{
    isix_assert_isr();
	atomic_store(&csys.jiffies, 0 );			//Zero jiffies if it was previously run
	/* The port sets schrun when the first task is entered */
	atomic_init( &csys.critical_count, 0 );
	//Restore context and run OS
	currp->state = OSTHR_STATE_RUNNING;
	_isix_port_start_first_task();
#if !(CONFIG_ISIX_SHUTDOWN_API)
	while(1);    //Prevent compiler warning
#endif
}


//! Reschedule tasks if it can be rescheduled
void _isixp_do_reschedule( ostask_t task )
{
	bool yield = false;
	if( !task ) {
		isix_bug("Unable to resched itself");
	}
    if( !schrun )
	{
        //Scheduler not running assign task
        if( !currp ) {
			currp = task;
		}
        else if(_isixp_prio_gt(task->prio,currp->prio)) {
			currp = task;
		}
    }
	else
	{
		if( _isixp_prio_gt(task->prio,currp->prio) ) {
			//New task have higer priority then current task
			pr_debug("resched: prio %i>old prio %i",task->prio,currp->prio);
			yield = true;
		}
	}
	isix_exit_critical();
	if(yield) _isix_port_yield();
}

static void wakeup_task( ostask_t task, osmsg_t msg )
{
	// Store the message retrived by remote
	task->obj.dmsg = msg;
	//If is still on time list
	if( list_is_elem_assigned( &task->inode_time ) ) {
		list_delete(&task->inode_time);
	}
	add_ready_list( task );
}

//! Wakeup task with selected message
void _isixp_wakeup_task( ostask_t task, osmsg_t msg )
{
	wakeup_task( task, msg );
	_isixp_do_reschedule( task );
}

//Wakeup but don't reschedule but exit critical
void _isixp_wakeup_task_i( ostask_t task, osmsg_t msg )
{
	wakeup_task( task, msg );
	_isixp_exit_critical_isr( task );
}

//Leave the critical section in the ISR path, yield when the woken task has the higher priority
void _isixp_exit_critical_isr( ostask_t woken )
{
#if CONFIG_ISIX_TICKLESS
	if( woken ) {
		catch_up_missed_ticks( _isix_port_systimer_resync() );
	}
#endif
	isix_exit_critical();
	if( woken && schrun && currp && _isixp_prio_gt(woken->prio, currp->prio) ) {
		_isix_port_yield();
	}
}

//Wakeup but don't reschedule but not unlock
void _isixp_wakeup_task_l( ostask_t task, osmsg_t msg )
{
	wakeup_task( task, msg );
}

//Delete task from ready list
void _isixp_set_sleep( thr_state_t newstate )
{
	pr_debug("gts: task %p new_state %i", currp, newstate );
#if CONFIG_ISIX_TEST_HOOKS
	if( schrun && (atomic_load(&csys.critical_count) != 1 ||
		_isix_port_atomic_sem_read_val(&csys.sched_lock)) ) {
		isix_bug( "Blocking call with critical section or scheduler lock held" );
	}
#endif
	delete_from_ready_list( currp );
	currp->state = newstate;
}

void _isixp_set_sleep_timeout( thr_state_t newstate, ostick_t timeout )
{
	pr_debug("gtsto: task %p new_state %i tout %i", currp ,newstate, timeout );
	_isixp_set_sleep( newstate );
	if( timeout != ISIX_TIME_INFINITE ) {
		add_task_to_waiting_list( currp, timeout );
	}
}

//! Reallocate according to priority change
void _isixp_reallocate_priority( ostask_t task, int newprio )
{
	if( task->state == OSTHR_STATE_READY ||
		task->state == OSTHR_STATE_RUNNING )
	{
		delete_from_ready_list( task );
		task->prio = newprio;
		task->state = OSTHR_STATE_SCHEDULE;
		add_ready_list( task );
	} else if( task->state == OSTHR_STATE_WTSEM ) {
		list_delete( &task->inode );
		task->prio = newprio;
		_isixp_add_to_prio_queue( &task->obj.sem->wait_list, task );
	} else if( task->state == OSTHR_STATE_WTMTX ) {
		list_delete( &task->inode );
		task->prio = newprio;
		_isixp_add_to_prio_queue( &task->obj.mtx->wait_list, task );
	} else if( task->state == OSTHR_STATE_WTCOND ) {
		list_delete( &task->inode );
		task->prio = newprio;
		_isixp_add_to_prio_queue( &task->obj.cond->wait_list, task );
	}
	else {
		task->prio = newprio;
	}
}

//TODO: Conditional variable / mutex cleanup
//Add task list to delete
void _isixp_add_kill_or_set_suspend( ostask_t task, bool suspend )
{
	// Remove task from timing list
	if( list_is_elem_assigned( &task->inode_time ) )
	{
		list_delete( &task->inode_time );
	}
	if( task->state==OSTHR_STATE_READY ||
		task->state==OSTHR_STATE_RUNNING )
	{
		delete_from_ready_list( task );
	}
	// If if task wait for sem
	if( task->state == OSTHR_STATE_WTEVT )
	{
		list_delete( &task->inode );
		task->wait_aborted = suspend;
	}
	else if( task->state == OSTHR_STATE_WTSEM )
	{
		_isixp_sem_fast_signal( task->obj.sem );
		list_delete( &task->inode );
		task->wait_aborted = suspend;
	}
	else if( task->state == OSTHR_STATE_WTCOND ) {
		//NOTE: Locked mutex will be released
		list_delete( &task->inode );
	}
	else if( task->state == OSTHR_STATE_WTMTX )
	{
		const osmtx_t mutex = task->obj.mtx;
		list_delete( &task->inode );
		_isixp_mutex_waiter_removed( mutex );
		task->wait_aborted = suspend;
	}
	else if( task->state == OSTHR_STATE_WTEXIT )
	{
		list_delete( &task->inode );
		task->wait_aborted = suspend;
	}
	if( suspend )
	{
		task->state = OSTHR_STATE_SUSPEND;
	}
	else
	{
		task->state = OSTHR_STATE_ZOMBIE;
		//Prepare to kill remove from time list
		list_insert_end( &csys.zombie_list,&task->inode );
		csys.number_of_task_deleted++;
	}
	__sync_synchronize();
}
