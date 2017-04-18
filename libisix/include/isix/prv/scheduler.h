#pragma once

#include <sys/reent.h>
#include <isix/prv/types.h>
#include <isix/prv/list.h>
#include <isix/config.h>
#include <isix/semaphore.h>
#include <isix/mutex.h>
#include <isix/condvar.h>
#include <isix/events.h>
#include <isix/scheduler.h>
#include <isix/arch/sem_atomic.h>
#include <isix/arch/memprot.h>
#include <isix/osthr_state.h>
#include <stdatomic.h>

#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

//Definition of task ready list
typedef struct task_ready_struct
{
    osprio_t prio;             //! Tasks group priority
	list_entry_t task_list;    //! List of task with equals priorities
    list_t inode;              //! List inode
} task_ready_t;


typedef uint8_t thr_state_t;

//Definition of task operations
struct isix_task
{
    unsigned long *top_stack;		//! Task stack ptr
    void *init_stack;				//! Initial value of stack for isix_free
#if CONFIG_ISIX_MEMORY_PROTECTION_MODEL > 0
	uintptr_t fence_estack;			//! Electric fence stack protector base
#endif
    osprio_t prio;					//! Priority of task
	osprio_t real_prio;				//! Real non inherited priority mtx
	osref_t  refcnt;				//! Reference counter for task deletion
	list_entry_t owned_mutexes;		//! Owned mutexes list
	list_entry_t waiting_tasks;		//! Tasks waiting for termination
    thr_state_t state;				//! Thread state
    ostick_t jiffies;				//! Ticks when task wake up
    task_ready_t *prio_elem;		//! Pointer to own prio list
	union
	{
		ossem_t sem;				//! Pointer to waiting sem
		osmtx_t mtx;				//! Pointer to waiting mutex
		osbitset_t evbits;			//! Current event bit for waiting
		oscondvar_t cond;			//! Conditional variable wait
		osmsg_t	dmsg;				//! Returning message
	} obj;
	struct _reent *impure_data;		//! Newlib per thread private data
    list_t inode;					//! Inode task for operation
	list_t inode_time;				//! Waiting inode
};

//!Structure related to isix system
struct isix_system
{
	_isix_port_atomic_sem_t sched_lock;		//! Schedule lock
	atomic_int critical_count;			//! Sched lock counter
	list_entry_t ready_list;			//! Binary tree of task ready to execute
	list_entry_t wait_lists[2];			//! Task waiting for event
	list_entry_t* p_wait_list;			//! Normal waiting task
	list_entry_t* pov_wait_list;		//! Overflow waiting tasks
	list_entry_t zombie_list;			//! Task waiting for event
	list_entry_t free_prio_elem;        //! Free priority innodes
	atomic_uint jiffies;				//! Global jiffies var
	atomic_int jiffies_skipped;			//! Skiped jiffies when scheduler is locked
	unsigned number_of_task_deleted;	//! Number of deleted task
	atomic_bool yield_pending;			//! Yield during lock
};

//Current executed task
extern struct isix_task *volatile _isix_current_task;
//Current task pointer
extern volatile bool _isix_scheduler_running;
#define currp _isix_current_task
#define schrun _isix_scheduler_running
//Return scheduler highest prio
static inline __attribute__((always_inline))
	ostask_t _isixp_max_prio( ostask_t t1, ostask_t t2 )
{
	return (t1->prio>t2->prio)?(t2):(t1);
}
//! Return true if fist prio is greater than second
static inline __attribute__((always_inline))
	bool _isixp_prio_gt( osprio_t p1, osprio_t p2 )
{
	return p1 < p2;
}

//! Return true if it is idle task
static inline __attribute__((always_inline))
bool _isixp_is_idle_prio( osprio_t p ) {
	return p==CONFIG_ISIX_NUMBER_OF_PRIORITIES;
}


//Scheduler function called on context switch in IRQ and Yield
void _isixp_schedule(void);
//Sched timer cyclic call
void _isixp_schedule_time(void);
//Process base stack initialization
unsigned long* _isixp_task_init_stack( unsigned long *sp, task_func_ptr_t pfun, void *param );
//Lock the scheduler
void _isixp_lock_scheduler();
//Unlock the scheduler
void _isixp_unlock_scheduler();
//! Wakeup task with selected message
void _isixp_wakeup_task( ostask_t task, osmsg_t msg );
void _isixp_wakeup_task_i( ostask_t task, osmsg_t msg );
void _isixp_wakeup_task_l( ostask_t task, osmsg_t msg );
//Add task list to delete
void _isixp_add_kill_or_set_suspend( ostask_t task, bool suspend );
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
void _isixp_do_reschedule( ostask_t task );
//! Terminate the task
void __attribute__((noreturn)) _isixp_task_terminator(void);
