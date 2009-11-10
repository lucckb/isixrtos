#include <isix/config.h>
#include <isix/types.h>
#include <prv/scheduler.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <prv/semaphore.h>
#include <string.h>

#ifndef ISIX_DEBUG_TASK
#define ISIX_DEBUG_TASK ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_TASK == ISIX_DBG_ON
#include <isix/printk.h>
#else
#define printk(...)
#endif

/*-----------------------------------------------------------------------*/
//Align Mask
#define ALIGN_MASK 0x03
//Align Bytes
#define ALIGN_BYTES 4
/*-----------------------------------------------------------------------*/
/* Create task function */
task_t* isix_task_create(task_func_ptr_t task_func, void *func_param, unsigned long  stack_depth, prio_t priority)
{
    printk("TaskCreate: Create task with prio %d\n",priority);
    //If stack length is small error
    if(stack_depth<PORT_SCHED_MIN_STACK_DEPTH) return NULL;
    //Alignement
    if(stack_depth & ALIGN_MASK)
    {
        stack_depth += ALIGN_BYTES - (stack_depth & ALIGN_MASK);
    }
    //Allocate task_t structure
    task_t *task = (task_t*)isix_alloc(sizeof(task_t));
    printk("TaskCreate: Alloc task struct %08x\n",task);
    //No free memory
    if(task==NULL) return NULL;
    //Zero task structure
    memset(task,0,sizeof(task_t));
    //Try Allocate stack for task
    task->init_stack = isix_alloc(stack_depth);
    printk("TaskCreate: Alloc stack mem %08x\n",task->init_stack);
    if(task->init_stack==NULL)
    {
        //Free allocated stack memory
        isix_free(task);
        return NULL;
    }
#ifdef CONFIG_STACK_GROWTH
     task->top_stack = (unsigned long*)((char*)task->init_stack + stack_depth - 4);
#else
     task->top_stack = task->init_stack;
#endif
    memset(task->init_stack,0x55,stack_depth);
    printk("TaskCreate: Top stack SP=%08x\n",task->top_stack);
    //Assign task priority
    task->prio = priority;
    //Task is ready
    task->state = TASK_READY;
    //Create initial task stack context
    task->top_stack = isixp_task_init_stack(task->top_stack,task_func,func_param);
    //Lock scheduler
    isixp_enter_critical();
    //Add task to ready list
    if(isixp_add_task_to_ready_list(task)<0)
    {
        //Free allocated innode
        printk("TaskCreate: Add task to ready list failed\n");
        isix_free(task->top_stack);
        isix_free(task);
	    isixp_exit_critical();
	    return NULL;
    }
    if(isix_scheduler_running==false)
    {
        //Scheduler not running assign task
        if(isix_current_task==NULL) isix_current_task = task;
        else if(isix_current_task->prio>task->prio) isix_current_task = task;
    }
    isixp_exit_critical();
    if(isix_current_task->prio>task->prio && isix_scheduler_running==true)
    {
        //New task have higer priority then current task
	    printk("TaskCreate: Call scheduler new prio %d > old prio %d\n",task->prio,isix_current_task->prio);
        isix_sched_yield();
    }
    return task;
}

/*-----------------------------------------------------------------------*/
/*Change task priority function
 * task - task pointer structure if NULL current prio change
 * new_prio - new priority                                  */
int isixp_task_change_prio(task_t *task,prio_t new_prio,bool yield)
{
    isixp_enter_critical();
    task_t *taskc = task?task:isix_current_task;
    //Save task prio
    prio_t prio = taskc->prio;
    if(prio==new_prio)
    {
        isixp_exit_critical();
        return ISIX_EOK;
    }
    bool yield_req = false;
    if(taskc->state & TASK_READY)
    {
        printk("ChangePrio: change prio of ready task\n");
        isixp_delete_task_from_ready_list(taskc);
        //Assign new prio
        taskc->prio = new_prio;
        //Add task to ready list
        if(isixp_add_task_to_ready_list(taskc)<0)
        {
            isixp_exit_critical();
            return ISIX_ENOMEM;
        }
        if(new_prio<prio && !(isix_current_task->state&TASK_RUNNING) ) yield_req = true;
    }
    else if(taskc->state & TASK_WAITING)
    {
        printk("ChangePrio: change prio of task waiting on sem\n");
        list_delete(&taskc->inode_sem);
        //Assign new prio
        taskc->prio = new_prio;
        isixp_add_task_to_sem_list(&taskc->sem->sem_task,taskc);
    }
    isixp_exit_critical();
    //Yield processor
    if(yield_req && yield)
    {
        printk("ChangePrio: CPUYield request\n");
        isix_sched_yield();
    }
    printk("ChangePrio: New prio %d\n",new_prio);
    return ISIX_EOK;
}

/*-----------------------------------------------------------------------*/
//Delete task pointed by struct task
int isix_task_delete(task_t *task)
{
    isixp_enter_critical();
    task_t *taskd = task?task:isix_current_task;
    if(taskd->state & TASK_READY)
    {
       //Task is ready remove from read
        isixp_delete_task_from_ready_list(taskd);
        printk("TaskDel: Remove from ready list\n");
    }
    else if(taskd->state & TASK_SLEEPING)
    {
        //Task sleeping remove from sleeping
        list_delete(&taskd->inode);
        printk("TaskDel: Remove from sleeping list\n");
    }
    //Task waiting for sem remove from waiting list
    if(taskd->state & TASK_WAITING)
    {
       list_delete(&taskd->inode_sem);
       taskd->sem = NULL;
       printk("TaskDel: Remove from sem list\n");
    }
    //Add task to delete list
    taskd->state = TASK_DEAD;
    isixp_add_task_to_delete_list(taskd);
    if(task==NULL || task==isix_current_task)
    {
        isixp_exit_critical();
        printk("TaskDel: Current task yield req\n");
        isix_sched_yield();
        return ISIX_EOK;
    }
    else
    {
        isixp_exit_critical();
        return ISIX_EOK;
    }
}

/*-----------------------------------------------------------------------*/
//Get current thread handler
task_t * isix_task_self(void)
{
    task_t *t = isix_current_task;
    return t;
}


