#include <isix/config.h>
#include <isix/prv/scheduler.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <isix/prv/list.h>
#include <isix/prv/semaphore.h>
#include <isix/prv/irqtimers.h>

#ifndef ISIX_DEBUG_SCHEDULER
#define ISIX_DEBUG_SCHEDULER ISIX_DBG_OFF
#endif

#if ISIX_DEBUG_SCHEDULER == ISIX_DBG_ON
#include <isix/printk.h>
#else
#undef isix_printk
#define isix_printk(...) do {} while(0)
#endif

/*-----------------------------------------------------------------------*/
// Task reschedule lock for spinlock
static struct isix_system csys;
/*-----------------------------------------------------------------------*/
//Current task pointer
volatile bool _isix_scheduler_running;
/*-----------------------------------------------------------------------*/
//Current task pointer
task_t * volatile _isix_current_task = NULL;
/*-----------------------------------------------------------------------*/
//! Kernel panic callback function definition
void __attribute__((weak)) isix_kernel_panic_callback( const char* file, int line, const char *msg )
{
	(void)file; (void)line; (void)msg;
}
/*-----------------------------------------------------------------------*/
//Isix bug report when isix_printk is defined
void isix_kernel_panic( const char *file, int line, const char *msg )
{
    //Go to critical sections forever
	_isixp_enter_critical();
#if ISIX_DEBUG_SCHEDULER
	isix_printk("OOPS-PANIC: Please reset board %s:%i [%s]", file, line, msg );
    task_ready_t *i;
    task_t *j;
    //TODO: Add interrupt blocking
    isix_printk("Ready tasks");
    list_for_each_entry(&ready_task,i,inode)
    {
         isix_printk("\t* List inode %08x prio %d",(unsigned int)i,i->prio);
         list_for_each_entry(&i->task_list,j,inode)
         {
              isix_printk("\t\t-> task %08x prio %d state %d",j,j->prio,j->state);
         }
    }
    isix_printk("Sleeping tasks");
    list_for_each_entry(p_waiting_task,j,inode)
    {
        isix_printk("\t->Task: %08x prio: %d state %d jiffies %d",j,j->prio,j->state,j->jiffies);
    }
#endif
    isix_kernel_panic_callback( file, line, msg );
    while(1);
}
/*-----------------------------------------------------------------------*/
//Lock scheduler
void _isixp_enter_critical(void)
{
	if( port_atomic_inc( &csys.critical_count ) == 1 ) 
	{
		port_set_interrupt_mask();
	}
    port_flush_memory();
}

/*-----------------------------------------------------------------------*/
//Unlock scheduler
void _isixp_exit_critical(void)
{
	if( port_atomic_dec( &csys.critical_count ) == 0 )
    {
		port_clear_interrupt_mask();
    }
	port_flush_memory();
}

/*-----------------------------------------------------------------------*/
//Scheduler is called in switch context
/**
 * NOTE: The process not require _isixp_enter_critical because
 * it is protected itself by pend svc vector lock 
 */
void _isixp_schedule(void)
{
	if( port_atomic_sem_read_val( &csys.sched_lock ) )
	{
		return;
	}
    //Remove executed task and add at end
    if(_isix_current_task->state & TASK_READY)
    {
        _isix_current_task->state &= ~TASK_RUNNING;
        list_delete(&_isix_current_task->inode);
        list_insert_end(&_isix_current_task->prio_elem->task_list,&_isix_current_task->inode);
    }
    task_ready_t * current_prio;
    //Get first ready prio
    current_prio = list_get_first(&csys.ready_list,inode,task_ready_t);
    isix_printk("Scheduler: actual prio %d prio list %08x",current_prio->prio,current_prio);
    //Get first ready task
    isix_printk("Scheduler: prev task %08x",_isix_current_task);
    _isix_current_task = list_get_first(&current_prio->task_list,inode,task_t);
    _isix_current_task->state |= TASK_RUNNING;
	//Handle local thread errno
	if(_isix_current_task->impure_data ) 
	{
		_REENT = _isix_current_task->impure_data;
	}
	else
	{
		if( _REENT != _GLOBAL_REENT )
			_REENT = _GLOBAL_REENT;
	}
    if(_isix_current_task->prio != current_prio->prio)
    {
    	isix_bug("Task priority doesn't match to element priority");
    }
    isix_printk("Scheduler: new task %08x",_isix_current_task);
}

/*-----------------------------------------------------------------------*/
//Time call from isr
static void internal_schedule_time(void)
{
	//Increment sys tick
	csys.jiffies++;
	if(!_isix_scheduler_running)
	{
		return;
	}
	if(csys.jiffies == 0)
	{
	   list_entry_t *tmp = csys.p_wait_list;
	   csys.p_wait_list = csys.pov_wait_list;
	   csys.pov_wait_list = tmp;
	}

    task_t *task_c;

    while( !list_isempty(csys.p_wait_list) &&
    		csys.jiffies>=(task_c = list_get_first(csys.p_wait_list,inode,task_t))->jiffies
      )
    {
    	isix_printk("SchedulerTime: sched_time %d task_time %d",jiffies,task_c->jiffies);
        task_c->state &= ~TASK_SLEEPING;
        task_c->state |= TASK_READY;
        list_delete(&task_c->inode);
        if(task_c->state & TASK_WAITING)
        {
            if(!task_c->sem)
            {
            	isix_bug( "TASK waiting on empty sem");
            }
        	task_c->state &= ~TASK_SEM_WKUP;
			port_atomic_dec( &task_c->sem->sem_task_count );
            task_c->sem = NULL;
            task_c->state &= ~TASK_WAITING;
            list_delete(&task_c->inode_sem);
            isix_printk("SchedulerTime: Timeout delete from sem list");
        }
        if(_isixp_add_task_to_ready_list(task_c)<0)
        {
            isix_bug("Add task to ready list fail");
        }
    }
	//Handle timvtimers
    _isixp_vtimer_handle_time( csys.jiffies );
}
/*-----------------------------------------------------------------------*/
static void unused_func(void ) {}
void isix_systime_handler(void) __attribute__ ((weak, alias("unused_func")));
/*-----------------------------------------------------------------------*/
//Schedule time handled from timer context
void _isixp_schedule_time() 
{
	//Call isix system time handler if used
    isix_systime_handler();
    if( port_atomic_sem_read_val( &csys.sched_lock ) ) {
		port_atomic_inc( &csys.jiffies_skipped );
	} else {
		//Increment system ticks
		_isixp_enter_critical();
		//Internal schedule time
		internal_schedule_time();
		//Clear interrupt mask
		_isixp_exit_critical();
	}
}
/*-----------------------------------------------------------------------*/
//Try get task ready from free list if is not exist allocate memory
static task_ready_t *alloc_task_ready_t(void)
{
   task_ready_t *prio = NULL;
   if(list_isempty(&csys.free_prio_elem))
   {
       isix_bug("Priority list not available");
   }
   else
   {
        //Get element from list
        prio = list_get_first(&csys.free_prio_elem,inode,task_ready_t);
        list_delete(&prio->inode);
        prio->prio = 0;
        isix_printk("alloc_task_ready_t: get from list node 0x%08x",prio);
   }
   return prio;
}

/*-----------------------------------------------------------------------*/
//Try get task ready from free list if is not exist allocate memory
static inline void free_task_ready_t(task_ready_t *prio)
{
    list_insert_end(&csys.free_prio_elem,&prio->inode);
    isix_printk("free_task_ready_t move 0x%08x to unused list",prio);
}

/*-----------------------------------------------------------------------*/
//Add assigned task to ready list
int _isixp_add_task_to_ready_list(task_t *task)
{
    if(task->prio > csys.number_of_priorities)
    	return ISIX_ENOPRIO;
    //Find task equal entry
    task_ready_t *prio_i;
    list_for_each_entry(&csys.ready_list,prio_i,inode)
    {
        //If task equal entry is found add this task to end list
        if(prio_i->prio==task->prio)
        {
            isix_printk("AddTaskToReadyList: found prio %d equal node %08x",prio_i->prio,prio_i);
            //Set pointer to priority struct
            task->prio_elem = prio_i;
            //Add task at end of ready list
            list_insert_end(&prio_i->task_list,&task->inode);
            return 0;
        }
        else if(task->prio < prio_i->prio)
        {
           isix_printk("AddTaskToReadyList: Insert %d node %08x",prio_i->prio,prio_i);
           break;
        }
    }
    //Priority not found allocate priority node
    task_ready_t *prio_n = alloc_task_ready_t();
    //If malloc return NULL then failed
    if(prio_n==NULL) return ISIX_ENOMEM;
    //Assign priority
    prio_n->prio = task->prio;
    //Set pointer to priority struct
    task->prio_elem = prio_n;
    //Initialize and add at end of list
    list_init(&prio_n->task_list);
    list_insert_end(&prio_n->task_list,&task->inode);
    list_insert_before(&prio_i->inode,&prio_n->inode);
    isix_printk("AddTaskToReadyList: Add new node %08x with prio %d",prio_n,prio_n->prio);
    return ISIX_EOK;
}

/*-----------------------------------------------------------------------*/
//Delete task from ready list
void _isixp_delete_task_from_ready_list(task_t *task)
{
    //Scheduler lock
   list_delete(&task->inode);
   //Check for task on priority structure
   if(list_isempty(&task->prio_elem->task_list))
   {
        //Task list is empty remove element
        isix_printk("DeleteTskFromRdyLst: Remove prio list elem");
        list_delete(&task->prio_elem->inode);
        free_task_ready_t(task->prio_elem);
   }
}

/*-----------------------------------------------------------------------*/
//Move selected task to waiting list
void _isixp_add_task_to_waiting_list(task_t *task, tick_t timeout)
{
    //Scheduler lock
    task->jiffies = csys.jiffies + timeout;
    if(task->jiffies < csys.jiffies)
    {
    	//Insert on overflow waiting list in time order
    	task_t *waitl;
    	list_for_each_entry(csys.pov_wait_list,waitl,inode)
    	{
    	   if(task->jiffies<waitl->jiffies) break;
    	}
    	isix_printk("MoveTaskToWaiting: OVERFLOW insert in time list at %08x",&waitl->inode);
    	list_insert_before(&waitl->inode,&task->inode);
    }
    else
    {
    	//Insert on waiting list in time order no overflow
    	task_t *waitl;
    	list_for_each_entry(csys.p_wait_list,waitl,inode)
    	{
    	    if(task->jiffies<waitl->jiffies) break;
    	}
    	isix_printk("MoveTaskToWaiting: NO overflow insert in time list at %08x",&waitl->inode);
    	list_insert_before(&waitl->inode,&task->inode);
    }
}

/*--------------------------------------------------------------*/
//Add task to semaphore list
void _isixp_add_task_to_sem_list(list_entry_t *sem_list,task_t *task)
{
    //Insert on waiting list in time order
    task_t *taskl;
    list_for_each_entry(sem_list,taskl,inode_sem)
    {
    	if(task->prio<taskl->prio) break;
    }
    isix_printk("MoveTaskToSem: insert in time list at %08x",taskl);
    list_insert_before(&taskl->inode_sem,&task->inode_sem);
}
/*-----------------------------------------------------------------------*/
//Add task list to delete
void _isixp_add_task_to_delete_list(task_t *task)
{
    list_insert_end(&csys.zombie_list,&task->inode);
    csys.number_of_task_deleted++;
}

/*-----------------------------------------------------------------------*/
//Dead task are clean by this procedure called from idle task
//One idle call clean one dead tasks
static inline void cleanup_tasks(void)
{
    if( csys.number_of_task_deleted > 0 )
    {
        _isixp_enter_critical();
        if(!list_isempty(&csys.zombie_list))
        {
        	task_t *task_del = list_get_first(&csys.zombie_list,inode,task_t);
        	list_delete(&task_del->inode);
        	isix_printk( "Task to delete: %08x(SP %08x) PRIO: %d",
						task_del,task_del->init_stack,task_del->prio );
        	port_cleanup_task(task_del->top_stack);
        	isix_free(task_del->init_stack);
        	isix_free(task_del);
			if( task_del->impure_data ) isix_free( task_del->impure_data );
        	csys.number_of_task_deleted--;
        }
        _isixp_exit_critical();
    }
}
/*-----------------------------------------------------------------------*/
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

/*-----------------------------------------------------------------------*/
//Get currrent jiffies
tick_t isix_get_jiffies(void)
{
    return csys.jiffies;
}

/*-----------------------------------------------------------------------*/
/* Number of priorites assigned when OS start */
void isix_init(prio_t num_priorities)
{
	//Schedule lock count
	port_atomic_sem_init( &csys.sched_lock, 0, 1 );
	//Copy priority
	csys.number_of_priorities = num_priorities;
	//Init heap
	isix_alloc_init();
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
    for(int i=0; i<num_priorities+1; i++)
    {
    	list_insert_end(&csys.free_prio_elem,&prio[i].inode);
    }
    //Lower priority is the idle task
    isix_task_create(idle_task,NULL,ISIX_PORT_SCHED_MIN_STACK_DEPTH,num_priorities,0);
    //Initialize virtual timers infrastructure
    _isixp_vtimer_init();
}

/*-----------------------------------------------------------------------*/
/* This function start scheduler after main function */
#ifndef ISIX_CONFIG_SHUTDOWN_API
void isix_start_scheduler(void) __attribute__((noreturn));
#endif
void isix_start_scheduler(void)
{
   csys.jiffies = 0;		//Zero jiffies if it was previously run
   _isix_scheduler_running = true;
   //Restore context and run OS
   port_start_first_task();
#ifndef ISIX_CONFIG_SHUTDOWN_API
   while(1);    //Prevent compiler warning
#endif
}
/*-----------------------------------------------------------------------*/
//Get maxium available priority
prio_t isix_get_min_priority(void)
{
	return csys.number_of_priorities;
}
/*-----------------------------------------------------------------------*/
//Return scheduler active
bool isix_is_scheduler_active(void)
{
    return _isix_scheduler_running;
}

/*-----------------------------------------------------------------------*/
#ifdef ISIX_CONFIG_SHUTDOWN_API
/**
 * Shutdown scheduler and return to main
 * @note It can be called only a once just before
 * the system shutdown for battery power save
 */
void isix_shutdown_scheduler(void)
{
	_isix_scheduler_running = false;
	port_yield();
}
/*-----------------------------------------------------------------------*/
/** Function called at end of isix execution only
 * when shutdown API is enabled
 */
void _isixp_finalize() {
	cleanup_tasks();
}
/*-----------------------------------------------------------------------*/
#endif /* ISIX_CONFIG_SHUTDOWN_API  */
/*-----------------------------------------------------------------------*/
/** Temporary lock task reschedule */
void _isixp_lock_scheduler() 
{
	port_atomic_sem_inc( &csys.sched_lock );
}
/*-----------------------------------------------------------------------*/
/** Temporary unlock task reschedule */
void _isixp_unlock_scheduler() 
{
	if( port_atomic_sem_dec( &csys.sched_lock ) == 1 ) {
		_isixp_enter_critical();
		while( csys.jiffies_skipped.counter > 0 ) {
			internal_schedule_time();
			--csys.jiffies_skipped.counter;
		}
		_isixp_exit_critical();
	}
}
/*-----------------------------------------------------------------------*/
