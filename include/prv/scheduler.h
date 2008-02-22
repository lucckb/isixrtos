#ifndef __ISIX_PRV_SCHEDULER_H
#define __ISIX_PRV_SCHEDULER_H

#include <prv/list.h>
#include <isix/config.h>
#include <isix/semaphore.h>
#include <isix/scheduler.h>
#include <prv/asm/scheduler.h>

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
/*-----------------------------------------------------------------------*/

//Definition of task operations
struct task_struct
{
    reg_t *top_stack;	        //Task stack ptr
    reg_t *init_stack;          //Initial value of stack for kfree
    prio_t prio;			    //Priority of task
    u8 state;                   //stan watku
    u64 jiffies;                //Ticks when task wake up
    task_ready_t *prio_elem;    //Pointer to own prio list
    list_t inode_sem;           //Inode of semaphore
    sem_t   *sem;               //Pointer to waiting sem
    list_t inode;               //List of tasks
};

/*-----------------------------------------------------------------------*/
#include <isix/types.h>
#include <isix/scheduler.h>

/*-----------------------------------------------------------------------*/
//Current executed task
extern struct task_struct *volatile current_task;

/*-----------------------------------------------------------------------*/
//Current task pointer
extern volatile bool scheduler_running;

/*-----------------------------------------------------------------------*/
//Global jiffies
extern volatile u64 jiffies;

/*-----------------------------------------------------------------------*/
//Scheduler function called on context switch in IRQ and Yield
void schedule(void);

/*-----------------------------------------------------------------------*/
//Sched timer cyclic call
void schedule_time(void);

/*-----------------------------------------------------------------------*/
//Lock scheduler and disable selected interrupt
int sched_lock_interrupt(unsigned long mask);
#define sched_lock() sched_lock_interrupt(0)
/*-----------------------------------------------------------------------*/
//Lock scheduler and reenable selected interrupt;
int sched_unlock_interrupt(unsigned long mask);
#define sched_unlock() sched_unlock_interrupt(0)
/*-----------------------------------------------------------------------*/
//Add selected task to waiting list
void add_task_to_waiting_list(struct task_struct *task);

/*-----------------------------------------------------------------------*/
//Add assigned task to ready list
int add_task_to_ready_list(struct task_struct *task);

/*--------------------------------------------------------------*/
//Private add task to semaphore list
void add_task_to_sem_list(list_entry_t *sem_list,struct task_struct *task);

/*-----------------------------------------------------------------------*/
//Delete task from ready list
void delete_task_from_ready_list(struct task_struct *task);

/*-----------------------------------------------------------------------*/
//Add task list to delete
void add_task_to_delete_list(struct task_struct *task);

/*-----------------------------------------------------------------------*/

#endif

