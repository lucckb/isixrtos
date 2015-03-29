#pragma once

#include <sys/reent.h>
#include <isix/types.h>
#include <isix/prv/list.h>
#include <isix/config.h>
#include <isix/semaphore.h>
#include <isix/scheduler.h>
#include <isix/port_atomic.h>

//*--------------------------------------------------------------------*/
//Definition of task ready list
typedef struct task_ready_struct
{
    prio_t prio;               //Tasks group priority
	list_entry_t task_list;    //List of task with equals priorities
    list_t inode;              //List inode
} task_ready_t;

/*--------------------------------------------------------------------*/
//! Current thread state
enum thr_state 
{
	THR_STATE_READY,			//! Thread is on ready state
	THR_STATE_RUNNING,			//! Thread is in running state
	THR_STATE_SLEEPING,			//! Thread on sleeping state
	THR_STATE_WTSEM,			//! Wait for semaphore state
	THR_STATE_WTEXIT,			//! Wait for exit state
	THR_STATE_ZOMBIE			//! In zombie state just before exit
};

typedef uint8_t thr_state_t;
/*--------------------------------------------------------------------*/
//Definition of task operations
struct task_struct
{
    unsigned long *top_stack;		//Task stack ptr
    unsigned long *init_stack;      //Initial value of stack for isix_free
    prio_t prio;			    	//Priority of task
    thr_state_t state;        		//Thread state
    tick_t jiffies;            		//Ticks when task wake up
    task_ready_t *prio_elem;    	//Pointer to own prio list
	union 
	{
		sem_t   *sem;               	// !Pointer to waiting sem
		msg_t	dmsg;					//! Returning message
	} obj;
    void    *prv;					//Private data pointer for extra data
	struct _reent *impure_data;		//Newlib per thread private data
    list_t inode;               	//Inode task for operation
	list_t inode_time;				//Waiting inode
};
/*--------------------------------------------------------------------*/
//!Structure related to isix system
struct isix_system 
{
	_port_atomic_t sched_lock;	//! Schedule lock 
	_port_atomic_int_t critical_count; 	//! Sched lock counter
	list_entry_t ready_list; 			//! Binary tree of task ready to execute
	list_entry_t wait_lists[2]; 		//!Task waiting for event
	list_entry_t* p_wait_list;			//! Normal waiting task
	list_entry_t* pov_wait_list;		//! Overflow waiting tasks
	list_entry_t zombie_list; 			//Task waiting for event
	list_entry_t free_prio_elem;        //Free priority innodes
	tick_t jiffies; 	//Global jiffies var
	_port_atomic_int_t jiffies_skipped; //Skiped jiffies when scheduler is locked
	unsigned number_of_task_deleted;  //Number of deleted task
	prio_t number_of_priorities; //Number of priorities
};
/*--------------------------------------------------------------------*/
//Current executed task
extern struct task_struct *volatile _isix_current_task;

/*--------------------------------------------------------------------*/
//Current task pointer
extern volatile bool _isix_scheduler_running;

/*--------------------------------------------------------------------*/
//Scheduler function called on context switch in IRQ and Yield
void _isixp_schedule(void);

/*--------------------------------------------------------------------*/
//Sched timer cyclic call
void _isixp_schedule_time(void);

/*--------------------------------------------------------------------*/
//Lock scheduler and disable selected interrupt
void _isixp_enter_critical(void);

/*--------------------------------------------------------------------*/
//Lock scheduler and reenable selected interrupt;
void _isixp_exit_critical(void);

/*--------------------------------------------------------------------*/
//Add selected task to waiting list
//void _isixp_add_task_to_waiting_list(struct task_struct *task, tick_t timeout);

/*--------------------------------------------------------------------*/
//Add assigned task to ready list
//void _isixp_add_ready_list( task_t* task );

/* ------------------------------------------------------------------ */ 
//Delete task from ready list
//void _isixp_delete_task_from_ready_list(struct task_struct *task);

/*--------------------------------------------------------------------*/
//Add task list to delete
//void _isixp_add_task_to_delete_list(struct task_struct *task);

/*--------------------------------------------------------------------*/
//Process base stack initialization
unsigned long* _isixp_task_init_stack(unsigned long *sp,task_func_ptr_t pfun,void *param);
/*--------------------------------------------------------------------*/
//Lock the scheduler
void _isixp_lock_scheduler();
/*--------------------------------------------------------------------*/
//Unlock the scheduler
void _isixp_unlock_scheduler();
/*--------------------------------------------------------------------*/
//! Wakeup task with selected message
void _isixp_wakeup_task( task_t* task, msg_t msg );
void _isixp_wakeup_task_i( task_t* task, msg_t msg );
/*--------------------------------------------------------------------*/
//! Go to Sleep state
msg_t _isixp_goto_sleep_timeout( thr_state_t newstate, tick_t timeout );
/* ------------------------------------------------------------------ */ 
//! Go to thread sleep
void _isixp_goto_sleep( thr_state_t newstate );
/*-----------------------------------------------------------------------*/
//! Add to the prio list from head
void _isixp_add_to_prio_queue( list_entry_t* objlist, struct task_struct* task );
/* ------------------------------------------------------------------ */ 
//! Remove task from prio queue
task_t* _isixp_remove_from_prio_queue( list_entry_t* list );
/*-----------------------------------------------------------------------*/
//! Reschedule tasks 
void _isixp_do_reschedule();
/*-----------------------------------------------------------------------*/
