#include <isix/config.h>
#include <isix/types.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <isix/prv/list.h>
#include <isix/prv/softtimers.h>
#include <isix/prv/semaphore.h>
#include <isix/prv/osstats.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
#include <isix/port_memprot.h>
#include <stdatomic.h>

#ifdef ISIX_LOGLEVEL_SCHEDULER
#undef ISIX_CONFIG_LOGLEVEL 
#define ISIX_CONFIG_LOGLEVEL ISIX_LOGLEVEL_SCHEDULER
#endif
#include <isix/prv/printk.h>


//Current task simple def
static ISIX_TASK_FUNC(idle_task,p);
static void add_ready_list( ostask_t task );
static void cleanup_tasks(void);
static void internal_schedule_time(void);

// Task reschedule lock for spinlock
static struct isix_system csys;
//Current task pointer
volatile bool _isix_scheduler_running;
//Current task pointer
ostask_t volatile _isix_current_task;

//! Ununsed systick handler
static void unused_func(void ) {}
void isix_systime_handler(void) __attribute__ ((weak, alias("unused_func")));

//! Kernel panic callback function definition
void __attribute__((weak)) 
isix_kernel_panic_callback( const char* file, int line, const char *msg )
{
	(void)file; (void)line; (void)msg;
}

//Get currrent jiffies
ostick_t isix_get_jiffies(void)
{
    return csys.jiffies;
}

//Get maxium available priority
osprio_t isix_get_min_priority(void)
{
	return csys.number_of_priorities;
}

/** Temporary lock task reschedule */
void _isixp_lock_scheduler() 
{
	port_atomic_sem_inc( &csys.sched_lock );
}

/** Temporary unlock task reschedule */
void _isixp_unlock_scheduler() 
{
	if( port_atomic_sem_dec( &csys.sched_lock ) == 1 ) {
		isix_enter_critical();
		while( csys.jiffies_skipped.counter > 0 ) {
			internal_schedule_time();
			--csys.jiffies_skipped.counter;
		}
		if( csys.yield_pending ) {
			port_yield();
			csys.yield_pending = false;
		}
		isix_exit_critical();
	}
}

#ifdef ISIX_CONFIG_SHUTDOWN_API
/**
 * Shutdown scheduler and return to main
 * @note It can be called only  a once just before
 * the system shutdown for battery power save
 */
void isix_shutdown_scheduler(void)
{
	//Dropout the task prot region 
	schrun = false;
	port_memory_protection_reset_efence();
	port_yield();
}

/** Function called at end of isix execution only
 * when shutdown API is enabled
 */
void _isixp_finalize() 
{
	_isixp_vtimers_finalize();
	cleanup_tasks();
}
#endif /* ISIX_CONFIG_SHUTDOWN_API  */

//Lock scheduler
void isix_enter_critical(void)
{
	if( atomic_fetch_add( &csys.critical_count, 1 ) == 0 )
	{
		port_set_interrupt_mask();
	}
}

//Unlock scheduler
void isix_exit_critical(void)
{
	int res = atomic_fetch_sub( &csys.critical_count, 1 );
	if( res <= 1 ) {
		port_clear_interrupt_mask();
    } else if( res <= 0 ) {
		isix_bug("Invalid lock count");
	}
	port_flush_memory();
}


/* Number of priorites assigned when OS start */
void isix_init(osprio_t num_priorities)
{
	//Schedule lock count
	port_memory_protection_set_default_map();
	port_atomic_sem_init( &csys.sched_lock, 0, 1 );
	atomic_init( &csys.critical_count, 0 );
	//Copy priority
	csys.number_of_priorities = num_priorities;
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
    task_ready_t *prio = isix_alloc(sizeof(task_ready_t)*(num_priorities+1));
	if( !prio ) {
		isix_bug("Insufficient memory alloc priority list");
	}
    for(int i=0; i<num_priorities+1; ++i) {
    	list_insert_end(&csys.free_prio_elem,&prio[i].inode);
    }
    //Lower priority is the idle task
	if( !isix_task_create( idle_task,NULL,
			ISIX_PORT_SCHED_MIN_STACK_DEPTH,num_priorities,0 ) ) {
		isix_bug("Insufficient memory idle task");	
	}
}

//Isix bug report when printk is defined
void isix_kernel_panic( const char *file, int line, const char *msg )
{
    //Go to critical sections forever
	isix_enter_critical();
#if ISIX_CONFIG_LOGLEVEL!=ISIXLOG_OFF
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
	tiny_printf("curr %p:%i jiff %u Ready tasks\n", currp, currp->prio, csys.jiffies );
    list_for_each_entry(&csys.ready_list,i,inode)
    {
         tiny_printf("\t* List inode %p prio %i\n",i, i->prio );
         list_for_each_entry(&i->task_list,j,inode)
         {
              tiny_printf("\t\t-> task %p prio %i state %i\n",j,j->prio,j->state);
         }
    }
    tiny_printf("Waiting tasks\n");
    list_for_each_entry(csys.p_wait_list,j,inode_time)
    {
        tiny_printf("\t->Task: %p prio: %i state %i jiffies %i\n",
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
	if( port_atomic_sem_read_val(&csys.sched_lock) ) {
		csys.yield_pending = true;
		return;
	}
    //Remove executed task and add at end
	if( currp->state == OSTHR_STATE_RUNNING ) {
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
#	if ISIX_CONFIG_MEMORY_PROTECTION_MODEL > 0
	port_memory_protection_reset_efence( );
	port_memory_protection_set_efence( currp->fence_estack );
#	endif
	//Update statistics
	_isixp_schedule_update_statistics( csys.jiffies, 
			currp->prio==csys.number_of_priorities );
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

//Time call from isr
static void internal_schedule_time(void)
{
	//Increment sys tick
	csys.jiffies++;
	if(!schrun)
	{
		return;
	}
	if(csys.jiffies == 0)
	{
	   list_entry_t *tmp = csys.p_wait_list;
	   csys.p_wait_list = csys.pov_wait_list;
	   csys.pov_wait_list = tmp;
	}

    ostask_t task_c;

    while( !list_isempty(csys.p_wait_list) &&
		csys.jiffies >=
		(task_c=list_first_entry(csys.p_wait_list,inode_time,struct isix_task))->jiffies
	)
    {
    	pr_debug("schedtime: task %p jiffies %i task_time %i", task_c,csys.jiffies,task_c->jiffies);
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
		}
		add_ready_list( task_c );
    }
}


//Schedule time handled from timer context
void _isixp_schedule_time() 
{
	//Call isix system time handler if used
    isix_systime_handler();
    if( port_atomic_sem_read_val( &csys.sched_lock ) ) {
		port_atomic_inc( &csys.jiffies_skipped );
	} else {
		//Increment system ticks
		isix_enter_critical();
		//Internal schedule time
		internal_schedule_time();
		//Clear interrupt mask
		isix_exit_critical();
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
    if(task->prio > csys.number_of_priorities) {
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
        else if( isixp_prio_gt(task->prio,prio_i->prio) )
        {
           pr_debug("wkup: Insert prio %i node %p",prio_i->prio,prio_i);
           break;
        }
    }
    //Priority not found allocate priority node
    task_ready_t *prio_n = alloc_task_ready_t();
    prio_n->prio = task->prio; 			//Assign priority
    task->prio_elem = prio_n; 			//Set pointer to priority struct
    list_init(&prio_n->task_list); 		//Initialize and add at end of list
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
    task->jiffies = csys.jiffies + timeout;
    if(task->jiffies < csys.jiffies)
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
    	if ( isixp_prio_gt(task->prio,item->prio) ) break;
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
        isix_enter_critical();
        if(!list_isempty(&csys.zombie_list))
        {
        	ostask_t task_del = list_first_entry(&csys.zombie_list,inode,struct isix_task);
        	list_delete(&task_del->inode);
        	pr_info( "Task to delete: %p(SP %p) PRIO: %i",
						task_del,task_del->init_stack,task_del->prio );
        	port_cleanup_task(task_del->top_stack);
        	isix_free(task_del->init_stack);
			if( task_del->impure_data ) isix_free( task_del->impure_data );
			if( task_del->prv ) isix_free( task_del->prv );
        	isix_free(task_del);
        	csys.number_of_task_deleted--;
        }
        isix_exit_critical();
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
        //Call port specific idle
        port_idle_cpu();
#ifndef  ISIX_CONFIG_USE_PREEMPTION
        isix_yield();
#endif
    }
}

/* This function start scheduler after main function */
#ifndef ISIX_CONFIG_SHUTDOWN_API
void isix_start_scheduler(void) __attribute__((noreturn));
#endif
void isix_start_scheduler(void)
{
	csys.jiffies = 0;		//Zero jiffies if it was previously run
	schrun = true;
	atomic_init( &csys.critical_count, 0 );
	//Restore context and run OS
	currp->state = OSTHR_STATE_RUNNING;
	port_start_first_task();
#ifndef ISIX_CONFIG_SHUTDOWN_API
	while(1);    //Prevent compiler warning
#endif
}


//! Reschedule tasks if it can be rescheduled
void _isixp_do_reschedule( ostask_t task )
{
	if( !task ) {
		isix_bug("Unable to resched itself");
	}
    if( !schrun ) 
	{
        //Scheduler not running assign task
        if( !currp ) { 
			currp = task;
		}
        else if(isixp_prio_gt(task->prio,currp->prio)) {
			currp = task;
		}
    } 
	else 
	{
		if( isixp_prio_gt(task->prio,currp->prio) ) {
			//New task have higer priority then current task
			pr_debug("resched: prio %i>old prio %i",task->prio,currp->prio);
			port_yield();
		}
	}
	isix_exit_critical();
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
	isix_exit_critical();
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
		_isixp_remove_from_prio_queue( &task->obj.sem->wait_list );
		task->prio = newprio;
		_isixp_add_to_prio_queue( &task->obj.sem->wait_list, task );
	}
}

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
	}
	else if( task->state == OSTHR_STATE_WTSEM ) 
	{
		_isixp_sem_fast_signal( task->obj.sem );
		list_delete( &task->inode );
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
}
