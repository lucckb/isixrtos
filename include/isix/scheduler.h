#ifndef __ISIX_SCHEDULER_H
#define __ISIX_SCHEDULER_H

#include <isix/list.h>
#include <isix/config.h>
#include <isix/semaphore.h>
/*-----------------------------------------------------------------------*/
//Define idle priority value
#define SCHED_IDLE_PRIORITY 255

/*-----------------------------------------------------------------------*/
//Pointer to task function
typedef void (*task_func_ptr_t)(void*);

/*-----------------------------------------------------------------------*/
//Priority type
typedef u8 prio_t;

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
/*-----------------------------------------------------------------------*/

//Definition of task operations
typedef struct task_struct
{
    reg_t *top_stack;	        //Task stack ptr
    prio_t prio;			    //Priority of task
    u8 state;                   //stan watku
    time_t time;                //Ticks when task wake up
    task_ready_t *prio_elem;    //Pointer to own prio list
    list_t inode_sem;           //Inode of semaphore
    list_t inode;               //List of tasks
} task_t;

/*-----------------------------------------------------------------------*/

#include <asm/scheduler.h>

/*-----------------------------------------------------------------------*/
//Current executed task
extern task_t *volatile current_task;

/*-----------------------------------------------------------------------*/
//Current task pointer
extern volatile bool scheduler_running;

/*-----------------------------------------------------------------------*/
//Global scheler time
extern volatile time_t sched_time;

/*-----------------------------------------------------------------------*/
//Scheduler function called on context switch in IRQ and Yield
void scheduler(void);

/*-----------------------------------------------------------------------*/
//Sched timer cyclic call
void scheduler_time(void);

/*-----------------------------------------------------------------------*/
//Lock scheduler
int sched_lock(void);

/*-----------------------------------------------------------------------*/
//Lock scheduler
int sched_unlock(void);

/*-----------------------------------------------------------------------*/
//Add selected task to waiting list
void add_task_to_waiting_list(task_t *task);

/*-----------------------------------------------------------------------*/
//Add assigned task to ready list 
int add_task_to_ready_list(task_t *task);

/*-----------------------------------------------------------------------*/
//Yield processor 
#define sched_yield() cpu_yield()

/*-----------------------------------------------------------------------*/
//HZ value used as system ticks
#define HZ CONFIG_HZ
/*-----------------------------------------------------------------------*/
#endif

