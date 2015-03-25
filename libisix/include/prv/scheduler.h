#ifndef _ISIX_PRV_SCHEDULER_H
#define _ISIX_PRV_SCHEDULER_H

#include <sys/reent.h>
#include <prv/list.h>
#include <isix/config.h>
#include <isix/semaphore.h>
#include <isix/scheduler.h>

//*-----------------------------------------------------------------------*/
//Definition of task ready list
typedef struct task_ready_struct
{
    prio_t prio;               //Tasks group priority
    list_entry_t task_list;    //List of task with some priority
    list_t inode;              //List inode
} task_ready_t;

/*-----------------------------------------------------------------------*/
//Task state
#define TASK_READY (1<<0)          //Task is ready
#define TASK_SLEEPING (1<<1)       //Task is sleeping
#define TASK_WAITING (1<<2)        //Task is waiting
#define TASK_RUNNING (1<<3)        //Task is running
#define TASK_DEAD    (1<<4)        //Task is dead
#define TASK_SEM_WKUP (1<<5)       //After sem wakeup
/*-----------------------------------------------------------------------*/

//Definition of task operations
struct task_struct
{
    unsigned long *top_stack;		//Task stack ptr
    unsigned long *init_stack;      //Initial value of stack for isix_free
    prio_t prio;			    	//Priority of task
    uint8_t state;        			//stan watku
    tick_t jiffies;            		//Ticks when task wake up
    task_ready_t *prio_elem;    	//Pointer to own prio list
    list_t inode_sem;           	//Inode of semaphore
    sem_t   *sem;               	//Pointer to waiting sem
    void    *prv;					//Private data pointer for extra data
	struct _reent *impure_data;		//Newlib per thread private data
    list_t inode;               	//List of tasks
};
/*-----------------------------------------------------------------------*/

#include <isix/types.h>
#include <isix/scheduler.h>

/*-----------------------------------------------------------------------*/
//Current executed task
extern struct task_struct *volatile _isix_current_task;

/*-----------------------------------------------------------------------*/
//Current task pointer
extern volatile bool _isix_scheduler_running;


/*-----------------------------------------------------------------------*/
//Scheduler function called on context switch in IRQ and Yield
void _isixp_schedule(void);

/*-----------------------------------------------------------------------*/
//Sched timer cyclic call
void _isixp_schedule_time(void);

/*-----------------------------------------------------------------------*/
//Lock scheduler and disable selected interrupt
void _isixp_enter_critical(void);

/*-----------------------------------------------------------------------*/
//Lock scheduler and reenable selected interrupt;
void _isixp_exit_critical(void);

/*-----------------------------------------------------------------------*/
//Add selected task to waiting list
void _isixp_add_task_to_waiting_list(struct task_struct *task, tick_t timeout);

/*-----------------------------------------------------------------------*/
//Add assigned task to ready list
int _isixp_add_task_to_ready_list(struct task_struct *task);

/*--------------------------------------------------------------*/
//Private add task to semaphore list
void _isixp_add_task_to_sem_list(list_entry_t *sem_list,struct task_struct *task);

/*-----------------------------------------------------------------------*/
//Delete task from ready list
void _isixp_delete_task_from_ready_list(struct task_struct *task);

/*-----------------------------------------------------------------------*/
//Add task list to delete
void _isixp_add_task_to_delete_list(struct task_struct *task);

/*-----------------------------------------------------------------------*/
//Process base stack initialization
unsigned long* _isixp_task_init_stack(unsigned long *sp,task_func_ptr_t pfun,void *param);

/*-----------------------------------------------------------------------*/
//Lock the scheduler
void _isixp_lock_scheduler();
/*-----------------------------------------------------------------------*/
//Unlock the scheduler
void _isixp_unlock_scheduler();
/*-----------------------------------------------------------------------*/
#endif

