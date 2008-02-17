#include <isix/config.h>
#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/task.h>
#include <isix/memory.h>

#ifndef DEBUG_TASK
#define DEBUG_TASK DBG_OFF
#endif


#if DEBUG_TASK == DBG_ON
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
task_t* task_create(task_func_ptr_t task_func, void *func_param,reg_t stack_depth, prio_t priority)
{
    printk("TaskCreate: Create task with prio %d\n",priority);
    //If stack length is small error
    if(stack_depth<SCHED_MIN_STACK_DEPTH) return NULL;
    //Alignement
    if(stack_depth & ALIGN_MASK)
    {
        stack_depth += ALIGN_BYTES - (stack_depth & ALIGN_MASK);
    }
    //Allocate task_t structure
    task_t *task = (task_t*)kmalloc(sizeof(task_t));
    printk("TaskCreate: Alloc task struct %08x\n",task);
    //No free memory
    if(task==NULL) return NULL;
    //Zero task structure
    zero_memory(task,sizeof(task_t));
    //Try Allocate stack for task
    task->init_stack = (reg_t*)kmalloc(stack_depth);
    printk("TaskCreate: Alloc stack mem %08x\n",task->init_stack);
    if(task->init_stack==NULL)
    {
        //Free allocated stack memory
        kfree(task);
        return NULL;
    }
#ifdef CONFIG_STACK_GROWTH
     task->top_stack = (reg_t*)((char*)task->init_stack + stack_depth - 4);
#else
     task->top_stack = task->init_stack;
#endif
    printk("TaskCreate: Top stack SP=%08x\n",task->top_stack);
    //Assign task priority
    task->prio = priority;
    //Task is ready
    task->state = TASK_READY;
    //Create initial task stack context
    task->top_stack = task_init_stack(task->top_stack,task_func,func_param);
    //Lock scheduler
    sched_lock();
    //Add task to ready list
    if(add_task_to_ready_list(task)<0)
    {
        //Free allocated innode
        printk("TaskCreate: Add task to ready list failed\n");
        kfree(task->top_stack);
        kfree(task);
	    sched_unlock();
	    return NULL;
    }
    if(scheduler_running==false)
    {
        //Scheduler not running assign task
        if(current_task==NULL) current_task = task;
        else if(current_task->prio>task->prio) current_task = task;
    }
    sched_unlock();
    if(current_task->prio>task->prio && scheduler_running==true)
    {
        //New task have higer priority then current task
	    printk("TaskCreate: Call scheduler new prio %d > old prio %d\n",task->prio,current_task->prio);
        sched_yield();
    }
    return task;
}

/*-----------------------------------------------------------------------*/
/*Change task priority function
 * task - task pointer structure if NULL current prio change
 * new_prio - new priority                                  */
int __task_change_prio(task_t *task,prio_t new_prio,bool yield)
{
    sched_lock();
    task_t *taskc = task?task:current_task;
    //Save task prio
    prio_t prio = taskc->prio;
    if(prio==new_prio)
    {
        sched_unlock();
        return EOK;
    }
    bool yield_req = false;
    if(taskc->state & TASK_READY)
    {
        printk("ChangePrio: change prio of ready task\n");
        delete_task_from_ready_list(taskc);
        //Assign new prio
        taskc->prio = new_prio;
        //Add task to ready list
        if(add_task_to_ready_list(taskc)<0)
        {
            sched_unlock();
            return ENOMEM;
        }
        if(new_prio<prio && !(current_task->state&TASK_RUNNING) ) yield_req = true;
    }
    else if(taskc->state & TASK_WAITING)
    {
        printk("ChangePrio: change prio of task waiting on sem\n");
        list_delete(&taskc->inode_sem);
        //Assign new prio
        taskc->prio = new_prio;
        add_task_to_sem_list(&taskc->sem->sem_task,taskc);
    }
    sched_unlock();
    //Yield processor
    if(yield_req && yield)
    {
        printk("ChangePrio: CPUYield request\n");
        sched_yield();
    }
    printk("ChangePrio: New prio %d\n",new_prio);
    return EOK;
}

/*-----------------------------------------------------------------------*/
//Delete task pointed by struct task
int task_delete(task_t *task)
{
    sched_lock();
    task_t *taskd = task?task:current_task;
    if(taskd->state & TASK_READY)
    {
       //Task is ready remove from read
        delete_task_from_ready_list(taskd);
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
    add_task_to_delete_list(taskd);
    if(task==NULL || task==current_task)
    {
        sched_unlock();
        printk("TaskDel: Current task yield req\n");
        sched_yield();
        return EOK;
    }
    else
    {
        sched_unlock();
        return EOK;
    }
}

