#pragma once

#include <sys/reent.h>
#include <isix/types.h>
#include <isix/prv/list.h>
#include <isix/config.h>
#include <isix/semaphore.h>
#include <isix/scheduler.h>
#include <isix/port_atomic.h>

#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

//Definition of task ready list
typedef struct task_ready_struct
{
    osprio_t prio;               //Tasks group priority
	list_entry_t task_list;    //List of task with equals priorities
    list_t inode;              //List inode
} task_ready_t;


//! Current thread state
enum thr_state 
{
	THR_STATE_READY		= 0,			//! Thread is on ready state
	THR_STATE_RUNNING	= 1,			//! Thread is in running state
	THR_STATE_CREATED	= 2,			//! Task already created but 
	THR_STATE_SLEEPING	= 3,			//! Thread on sleeping state
	THR_STATE_WTSEM		= 4,			//! Wait for semaphore state
	THR_STATE_WTEXIT	= 5,			//! Wait for exit state
	THR_STATE_ZOMBIE	= 6,			//! In zombie state just before exit
	THR_STATE_SCHEDULE  = 7,			//! Schedule only do nothing special
};

typedef uint8_t thr_state_t;

//Definition of task operations
struct isix_task
{
    unsigned long *top_stack;		//Task stack ptr
    unsigned long *init_stack;      //Initial value of stack for isix_free
    osprio_t prio;			    	//Priority of task
    thr_state_t state;        		//Thread state
    ostick_t jiffies;            		//Ticks when task wake up
    task_ready_t *prio_elem;    	//Pointer to own prio list
	union 
	{
		ossem_t sem;               		// !Pointer to waiting sem
		osmsg_t	dmsg;					//! Returning message
	} obj;
    void    *prv;					//Private data pointer for extra data
	struct _reent *impure_data;		//Newlib per thread private data
    list_t inode;               	//Inode task for operation
	list_t inode_time;				//Waiting inode
};

//!Structure related to isix system
struct isix_system 
{
	_port_atomic_t sched_lock;			//! Schedule lock 
	_port_atomic_int_t critical_count; 	//! Sched lock counter
	list_entry_t ready_list; 			//! Binary tree of task ready to execute
	list_entry_t wait_lists[2]; 		//!Task waiting for event
	list_entry_t* p_wait_list;			//! Normal waiting task
	list_entry_t* pov_wait_list;		//! Overflow waiting tasks
	list_entry_t zombie_list; 			//Task waiting for event
	list_entry_t free_prio_elem;        //Free priority innodes
	ostick_t jiffies; 					//Global jiffies var
	_port_atomic_int_t jiffies_skipped; //Skiped jiffies when scheduler is locked
	unsigned number_of_task_deleted;  	//Number of deleted task
	osprio_t number_of_priorities; 		//Number of priorities
};

//Current executed task
extern struct isix_task *volatile _isix_current_task;
//Current task pointer
extern volatile bool _isix_scheduler_running;
#define currp _isix_current_task
#define schrun _isix_scheduler_running

//Scheduler function called on context switch in IRQ and Yield
void _isixp_schedule(void);
//Sched timer cyclic call
void _isixp_schedule_time(void);
//Lock scheduler and disable selected interrupt
void _isixp_enter_critical(void);
//Lock scheduler and reenable selected interrupt;
void _isixp_exit_critical(void);
//Process base stack initialization
unsigned long* _isixp_task_init_stack( unsigned long *sp, task_func_ptr_t pfun, void *param );
//Lock the scheduler
void _isixp_lock_scheduler();
//Unlock the scheduler
void _isixp_unlock_scheduler();
//! Wakeup task with selected message
void _isixp_wakeup_task( ostask_t task, osmsg_t msg );
void _isixp_wakeup_task_i( ostask_t task, osmsg_t msg );
//Add task list to delete
void _isixp_add_to_kill_list( ostask_t task );
//! Set sleep state but not reschedule
void _isixp_set_sleep_timeout( thr_state_t newstate, ostick_t timeout );
//! Go to thread sleep
void _isixp_set_sleep( thr_state_t newstate );
//! Add to the prio list from head
void _isixp_add_to_prio_queue( list_entry_t* objlist, struct isix_task* task );
//! Remove task from prio queue
ostask_t _isixp_remove_from_prio_queue( list_entry_t* list );
//! Reallocate according to priority change
void _isixp_reallocate_priority( ostask_t task, int newprio );
//! Reschedule tasks 
void _isixp_do_reschedule();

