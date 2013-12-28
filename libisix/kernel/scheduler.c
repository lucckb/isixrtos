#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <prv/scheduler.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <prv/list.h>
#include <prv/semaphore.h>
#include <prv/irqtimers.h>
#include <prv/multiple_objects.h>

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
//! Kernel panic callback function definition
void __attribute__((weak)) isix_kernel_panic_callback( const char* file, int line, const char *msg )
{
	(void)file; (void)line; (void)msg;
}
/*-----------------------------------------------------------------------*/
//Current task pointer
volatile bool isix_scheduler_running;

/*-----------------------------------------------------------------------*/
//Current task pointer
task_t * volatile isix_current_task = NULL;

/*-----------------------------------------------------------------------*/
//Sched lock counter
static _port_atomic_int_t critical_count;

/*-----------------------------------------------------------------------*/
// Task reschedule lock for spinlock
static _port_atomic_t sem_schedule_lock = { 0, 1 };
/*-----------------------------------------------------------------------*/
//Binary tree of task ready to execute
static list_entry_t ready_task;

/*-----------------------------------------------------------------------*/
//Task waiting for event
static list_entry_t wait_tasks[2];

//Pointer to current list overflow list
static list_entry_t* p_waiting_task;
static list_entry_t* pov_waiting_task;

/*-----------------------------------------------------------------------*/
//Task waiting for event
static list_entry_t dead_task;

/*-----------------------------------------------------------------------*/
//Free priority innodes
static list_entry_t free_prio_elem;

/*-----------------------------------------------------------------------*/
//Global jiffies var
static volatile tick_t jiffies;
//Skiped jiffies when scheduler is locked
static _port_atomic_int_t jiffies_skipped;
//Number of deleted task
static volatile unsigned number_of_task_deleted;
//Number of priorities
static volatile prio_t number_of_priorities;

/*-----------------------------------------------------------------------*/
//Isix bug report when isix_printk is defined
void isix_kernel_panic( const char *file, int line, const char *msg )
{
    //Go to critical sections forever
	isixp_enter_critical();
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
void isixp_enter_critical(void)
{
	if( port_atomic_inc( &critical_count ) == 1 ) 
	{
		port_set_interrupt_mask();
	}
    port_flush_memory();
}

/*-----------------------------------------------------------------------*/
//Unlock scheduler
void isixp_exit_critical(void)
{
	if( port_atomic_dec( &critical_count ) == 0 )
    {
		port_clear_interrupt_mask();
    }
	port_flush_memory();
}

/*-----------------------------------------------------------------------*/
//Scheduler is called in switch context
/**
 * NOTE: The process not require isixp_enter_critical because
 * it is protected itself by pend svc vector lock 
 */
void isixp_schedule(void)
{
	if( port_atomic_sem_read_val( &sem_schedule_lock ) )
	{
		return;
	}
    //Remove executed task and add at end
    if(isix_current_task->state & TASK_READY)
    {
        isix_current_task->state &= ~TASK_RUNNING;
        list_delete(&isix_current_task->inode);
        list_insert_end(&isix_current_task->prio_elem->task_list,&isix_current_task->inode);
    }
    task_ready_t * current_prio;
    //Get first ready prio
    current_prio = list_get_first(&ready_task,inode,task_ready_t);
    isix_printk("Scheduler: actual prio %d prio list %08x",current_prio->prio,current_prio);
    //Get first ready task
    isix_printk("Scheduler: prev task %08x",isix_current_task);
    isix_current_task = list_get_first(&current_prio->task_list,inode,task_t);
    isix_current_task->state |= TASK_RUNNING;
    if(isix_current_task->prio != current_prio->prio)
    {
    	isix_bug("Task priority doesn't match to element priority");
    }
    isix_printk("Scheduler: new task %08x",isix_current_task);
}

/*-----------------------------------------------------------------------*/
//Time call from isr
static void internal_schedule_time(void)
{
	//Increment sys tick
	jiffies++;
	if(!isix_scheduler_running)
	{
		return;
	}
	if(jiffies == 0)
	{
	   list_entry_t *tmp = p_waiting_task;
	   p_waiting_task = pov_waiting_task;
	   pov_waiting_task = tmp;
	}

    task_t *task_c;

    while( !list_isempty(p_waiting_task) &&
    		jiffies>=(task_c = list_get_first(p_waiting_task,inode,task_t))->jiffies
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
            task_c->sem = NULL;
            task_c->state &= ~TASK_WAITING;
            list_delete(&task_c->inode_sem);
            isix_printk("SchedulerTime: Timeout delete from sem list");
        }
#ifdef ISIX_CONFIG_USE_MULTIOBJECTS
        if(task_c->state & TASK_WAITING_MULTIPLE)
        {
        	task_c->state &= ~(TASK_WAITING_MULTIPLE|TASK_MULTIPLE_WKUP);
        }
#endif
        if(isixp_add_task_to_ready_list(task_c)<0)
        {
            isix_bug("Add task to ready list fail");
        }
    }
	//Handle timvtimers
    isixp_vtimer_handle_time( jiffies );
}
/*-----------------------------------------------------------------------*/
static void unused_func(void ) {}
void isix_systime_handler(void) __attribute__ ((weak, alias("unused_func")));
/*-----------------------------------------------------------------------*/
//Schedule time handled from timer context
void isixp_schedule_time() 
{
	//Call isix system time handler if used
    isix_systime_handler();
    if( port_atomic_sem_read_val( &sem_schedule_lock ) ) {
		port_atomic_inc( &jiffies_skipped );
	} else {
		//Increment system ticks
		isixp_enter_critical();
		//Internal schedule time
		internal_schedule_time();
		//Clear interrupt mask
		isixp_exit_critical();
	}
}
/*-----------------------------------------------------------------------*/
//Try get task ready from free list if is not exist allocate memory
static task_ready_t *alloc_task_ready_t(void)
{
   task_ready_t *prio = NULL;
   if(list_isempty(&free_prio_elem))
   {
       isix_bug("Priority list not available");
   }
   else
   {
        //Get element from list
        prio = list_get_first(&free_prio_elem,inode,task_ready_t);
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
    list_insert_end(&free_prio_elem,&prio->inode);
    isix_printk("free_task_ready_t move 0x%08x to unused list",prio);
}

/*-----------------------------------------------------------------------*/
//Add assigned task to ready list
int isixp_add_task_to_ready_list(task_t *task)
{
    if(task->prio > number_of_priorities)
    	return ISIX_ENOPRIO;
    //Find task equal entry
    task_ready_t *prio_i;
    list_for_each_entry(&ready_task,prio_i,inode)
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
void isixp_delete_task_from_ready_list(task_t *task)
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
void isixp_add_task_to_waiting_list(task_t *task, tick_t timeout)
{
    //Scheduler lock
    task->jiffies = jiffies + timeout;
    if(task->jiffies < jiffies)
    {
    	//Insert on overflow waiting list in time order
    	task_t *waitl;
    	list_for_each_entry(pov_waiting_task,waitl,inode)
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
    	list_for_each_entry(p_waiting_task,waitl,inode)
    	{
    	    if(task->jiffies<waitl->jiffies) break;
    	}
    	isix_printk("MoveTaskToWaiting: NO overflow insert in time list at %08x",&waitl->inode);
    	list_insert_before(&waitl->inode,&task->inode);
    }
}

/*--------------------------------------------------------------*/
//Add task to semaphore list
void isixp_add_task_to_sem_list(list_entry_t *sem_list,task_t *task)
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
void isixp_add_task_to_delete_list(task_t *task)
{
    list_insert_end(&dead_task,&task->inode);
    number_of_task_deleted++;
}

/*-----------------------------------------------------------------------*/
//Dead task are clean by this procedure called from idle task
//One idle call clean one dead tasks
static inline void cleanup_tasks(void)
{
    if( number_of_task_deleted > 0 )
    {
        isixp_enter_critical();
        if(!list_isempty(&dead_task))
        {
        	task_t *task_del = list_get_first(&dead_task,inode,task_t);
        	list_delete(&task_del->inode);
        	isix_printk("Task to delete: %08x(SP %08x) PRIO: %d",task_del,task_del->init_stack,task_del->prio);
        	port_cleanup_task(task_del->top_stack);
        	isix_free(task_del->init_stack);
        	isix_free(task_del);
        	number_of_task_deleted--;
        }
        isixp_exit_critical();
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
    return jiffies;
}

/*-----------------------------------------------------------------------*/
/* Number of priorites assigned when OS start */
void isix_init(prio_t num_priorities)
{
	//Copy priority
	number_of_priorities = num_priorities;
	//Init heap
	isix_alloc_init();
	//Initialize ready task list
    list_init(&ready_task);
    //Initialize waiting list
    list_init(&wait_tasks[0]);
    list_init(&wait_tasks[1]);
    //Initialize overflow waiting list
    p_waiting_task = &wait_tasks[0];
    pov_waiting_task = &wait_tasks[1];
    //Initialize dead task
    list_init(&dead_task);
    //Initialize free prio elem list
    list_init(&free_prio_elem);
    //This memory never will be freed
    task_ready_t *prio = isix_alloc(sizeof(task_ready_t)*(num_priorities+1));
    for(int i=0; i<num_priorities+1; i++)
    {
    	list_insert_end(&free_prio_elem,&prio[i].inode);
    }
    //Lower priority is the idle task
    isix_task_create(idle_task,NULL,ISIX_PORT_SCHED_MIN_STACK_DEPTH,num_priorities);
    //Initialize virtual timers infrastructure
    isixp_vtimer_init();
    //Initialize multiple objects infrastructure
    ixixp_multiple_objects_init();
}

/*-----------------------------------------------------------------------*/
/* This function start scheduler after main function */
#ifndef ISIX_CONFIG_SHUTDOWN_API
void isix_start_scheduler(void) __attribute__((noreturn));
#endif
void isix_start_scheduler(void)
{
   jiffies = 0;		//Zero jiffies if it was previously run
   isix_scheduler_running = true;
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
	return number_of_priorities;
}
/*-----------------------------------------------------------------------*/
//Return scheduler active
bool isix_is_scheduler_active(void)
{
    return isix_scheduler_running;
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
	isix_scheduler_running = false;
	port_yield();
}
/*-----------------------------------------------------------------------*/
/** Function called at end of isix execution onlt
 * when shutdown API is enabled
 */
void _isixp_finalize() {
	cleanup_tasks();
}
/*-----------------------------------------------------------------------*/
/** Temporary lock task reschedule */
void _isixp_lock_scheduler() 
{
	port_atomic_sem_inc( &sem_schedule_lock );
}
/*-----------------------------------------------------------------------*/
/** Temporary unlock task reschedule */
void _isixp_unlock_scheduler() 
{
	if( port_atomic_sem_dec( &sem_schedule_lock ) == 1 ) {
		isixp_enter_critical();
		while( jiffies_skipped.counter > 0 ) {
			internal_schedule_time();
			--jiffies_skipped.counter;
		}
		isixp_exit_critical();
	}
}
/*-----------------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------------*/
