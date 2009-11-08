#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <prv/scheduler.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <prv/list.h>
#include <prv/semaphore.h>

#ifndef ISIX_DEBUG_SCHEDULER
#define ISIX_DEBUG_SCHEDULER ISIX_DBG_OFF
#endif


/*-----------------------------------------------------------------------*/
//Current task pointer
volatile bool isix_scheduler_running;

/*-----------------------------------------------------------------------*/
//Current task pointer
task_t * volatile isix_current_task = NULL;

/*-----------------------------------------------------------------------*/
//Sched lock counter
static volatile int sched_lock_counter = 0;

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

//FIXME: Remove global after debug
volatile tick_t jiffies;

/*-----------------------------------------------------------------------*/
void isix_bug(void)
{
    printk("OOPS: Please reset board\n");
    task_ready_t *i;
    task_t *j;
    //TODO: Add interrupt blocking
    printk("Ready tasks\n");
    list_for_each_entry(&ready_task,i,inode)
    {
         printk("\t* List inode %08x prio %d\n",(unsigned int)i,i->prio);
         list_for_each_entry(&i->task_list,j,inode)
         {
              printk("\t\t-> task %08x prio %d state %d\n",j,j->prio,j->state);
         }
    }
    printk("Sleeping tasks\n");
    list_for_each_entry(p_waiting_task,j,inode)
    {
        printk("\t->Task: %08x prio: %d state %d jiffies %d\n",j,j->prio,j->state,j->jiffies);
    }
    while(1);
}

/*-----------------------------------------------------------------------*/
//After isix_bug function disable printk
#if ISIX_DEBUG_SCHEDULER == ISIX_DBG_ON
static void print_tasks(list_entry_t *sem_list);
#else
#undef printk
#define printk(...)
#define print_tasks(v)
#endif

/*-----------------------------------------------------------------------*/
//Lock scheduler
void isixp_sched_lock(void)
{
    port_set_interrupt_mask();
    sched_lock_counter++;
    port_clear_interrupt_mask();
}

/*-----------------------------------------------------------------------*/
//Unlock scheduler
void isixp_sched_unlock(void)
{
    port_set_interrupt_mask();
    if(sched_lock_counter>0) sched_lock_counter--;
    port_clear_interrupt_mask();
}

/*-----------------------------------------------------------------------*/
//Scheduler is called in switch context
void isixp_schedule(void)
{
    //If scheduler is locked switch context is disable
    if(sched_lock_counter) return;
    print_tasks(NULL);
    isixp_sched_lock();

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
    printk("Scheduler: actual prio %d prio list %08x\n",current_prio->prio,current_prio);
    //Get first ready task
    printk("Scheduler: prev task %08x\n",isix_current_task);
    isix_current_task = list_get_first(&current_prio->task_list,inode,task_t);
    isix_current_task->state |= TASK_RUNNING;
    if(isix_current_task->prio != current_prio->prio) isix_bug();
    printk("Scheduler: new task %08x\n",isix_current_task);
    isixp_sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Time call from isr
void isixp_schedule_time(void)
{
	//If scheduler is locked switch context is disable
	if(sched_lock_counter) return;

	//Increment sys tick
	jiffies++;
	if(jiffies == 0)
	{
	   list_entry_t *tmp = p_waiting_task;
	   p_waiting_task = pov_waiting_task;
	   pov_waiting_task = tmp;
	}

    //if(list_isempty(&waiting_task)) return;
    task_t *task_c;

    while( !list_isempty(p_waiting_task) &&
    		jiffies>=(task_c = list_get_first(p_waiting_task,inode,task_t))->jiffies
      )
    {
    	//if(jiffies<task_c->jiffies) return;
    	printk("SchedulerTime: sched_time %d task_time %d\n",jiffies,task_c->jiffies);
        task_c->state &= ~TASK_SLEEPING;
        task_c->state |= TASK_READY;
        list_delete(&task_c->inode);
        if(task_c->state & TASK_WAITING)
        {
            task_c->state &= ~TASK_WAITING;
            list_delete(&task_c->inode_sem);
            task_c->sem->sem_ret = ISIX_ETIMEOUT;
            task_c->sem = NULL;
            printk("SchedulerTime: Timeout delete from sem list\n");
        }
        if(isixp_add_task_to_ready_list(task_c)<0)
        {
            printk("SchedulerTime: Error in add task to ready list\n");
            isix_bug();
        }
    }
}
/*-----------------------------------------------------------------------*/
//Try get task ready from free list if is not exist allocate memory
static task_ready_t *alloc_task_ready_t(void)
{
   task_ready_t *prio = NULL;
   if(list_isempty(&free_prio_elem)==true)
   {
        //If no free elem allocate it
        prio = (task_ready_t*)isix_alloc(sizeof(task_ready_t));
        printk("alloc_task_ready_t: kmalloc() node %08x\n",prio);
   }
   else
   {
        //Get element from list
        prio = list_get_first(&free_prio_elem,inode,task_ready_t);
        list_delete(&prio->inode);
        prio->prio = 0;
        printk("alloc_task_ready_t: get from list node 0x%08x\n",prio);
   }
   return prio;
}

/*-----------------------------------------------------------------------*/
//Try get task ready from free list if is not exist allocate memory
static inline void free_task_ready_t(task_ready_t *prio)
{
    list_insert_end(&free_prio_elem,&prio->inode);
    printk("free_task_ready_t move 0x%08x to unused list\n",prio);
}

/*-----------------------------------------------------------------------*/
//Add assigned task to ready list
int isixp_add_task_to_ready_list(task_t *task)
{
    //Scheduler lock
    isixp_sched_lock();
    //Find task equal entry
    task_ready_t *prio_i;
    list_for_each_entry(&ready_task,prio_i,inode)
    {
        //If task equal entry is found add this task to end list
        if(prio_i->prio==task->prio)
        {
            printk("AddTaskToReadyList: found prio %d equal node %08x\n",prio_i->prio,prio_i);
            //Set pointer to priority struct
            task->prio_elem = prio_i;
            //Add task at end of ready list
            list_insert_end(&prio_i->task_list,&task->inode);
            //Unlock scheduler
            isixp_sched_unlock();
            return 0;
        }
        else if(prio_i->prio<task->prio)
        {
           printk("AddTaskToReadyList: Insert %d node %08x\n",prio_i->prio,prio_i);
           break;
        }
    }
    //Priority not found allocate priority node
    task_ready_t *prio_n = alloc_task_ready_t();
    //If malloc return NULL then failed
    if(prio_n==NULL) return -1;
    //Assign priority
    prio_n->prio = task->prio;
    //Set pointer to priority struct
    task->prio_elem = prio_n;
    //Initialize and add at end of list
    list_init(&prio_n->task_list);
    list_insert_end(&prio_n->task_list,&task->inode);
    list_insert_after(&prio_i->inode,&prio_n->inode);
    printk("AddTaskToReadyList: Add new node %08x with prio %d\n",prio_n,prio_n->prio);
    isixp_sched_unlock();
    return 0;
}

/*-----------------------------------------------------------------------*/
//Delete task from ready list
void isixp_delete_task_from_ready_list(task_t *task)
{
    //Scheduler lock
   isixp_sched_lock();
   list_delete(&task->inode);
   //Check for task on priority structure
   if(list_isempty(&task->prio_elem->task_list)==true)
   {
        //Task list is empty remove element
        printk("DeleteTskFromRdyLst: Remove prio list elem\n");
        list_delete(&task->prio_elem->inode);
        free_task_ready_t(task->prio_elem);
   }
   //Scheduler unlock
   isixp_sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Move selected task to waiting list
void isixp_add_task_to_waiting_list(task_t *task, tick_t timeout)
{
    //Scheduler lock
    isixp_sched_lock();
    task->jiffies = jiffies + timeout;
    if(task->jiffies < jiffies)
    {
    	//Insert on overflow waiting list in time order
    	task_t *waitl;
    	list_for_each_entry(pov_waiting_task,waitl,inode)
    	{
    	   if(task->jiffies<waitl->jiffies) break;
    	}
    	printk("MoveTaskToWaiting: OVERFLOW insert in time list at %08x\n",&waitl->inode);
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
    	printk("MoveTaskToWaiting: NO overflow insert in time list at %08x\n",&waitl->inode);
    	list_insert_before(&waitl->inode,&task->inode);
    }
    //Scheduler unlock
    isixp_sched_unlock();
}

/*--------------------------------------------------------------*/
//Add task to semaphore list
void isixp_add_task_to_sem_list(list_entry_t *sem_list,task_t *task)
{
    //Scheduler lock
    isixp_sched_lock();
    //Insert on waiting list in time order
    task_t *taskl;
    list_for_each_entry_reverse(sem_list,taskl,inode_sem)
    {
       if(task->prio<=taskl->prio) break;
    }
    printk("MoveTaskToSem: insert in time list at %08x\n",taskl);
    list_insert_after(&taskl->inode_sem,&task->inode_sem);
    print_tasks(sem_list);
    //Scheduler unlock
    isixp_sched_unlock();

}
/*-----------------------------------------------------------------------*/
//Add task list to delete
void isixp_add_task_to_delete_list(task_t *task)
{
    //lock scheduler
    isixp_sched_lock();
    list_insert_end(&dead_task,&task->inode);
    isixp_sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Dead task are clean by this procedure called from idle task
//One idle call clean one dead tasks
static inline void cleanup_tasks(void)
{
    if(list_isempty(&dead_task)==false)
    {
        isixp_sched_lock();
        task_t *task_del = list_get_first(&dead_task,inode,task_t);
        list_delete(&task_del->inode);
        printk("CleanupTasks: Task to delete is %08x stack SP %08x\n",task_del,task_del->init_stack);
        isix_free(task_del->init_stack);
        isix_free(task_del);
        //Remove one priority from free priority innodes
        if(list_isempty(&free_prio_elem)==false)
        {
            task_ready_t *free_prio = list_get_first(&free_prio_elem,inode,task_ready_t);
            list_delete(&free_prio->inode);
            isix_free(free_prio);
            printk("CleanupTasks: Free inode prio 0x%08x\n",free_prio);
        }
        isixp_sched_unlock();
    }
}

/*-----------------------------------------------------------------------*/
//Idle task function do nothing and lower priority
ISIX_TASK_FUNC(idle_task,p)
{
    while(1)
    {
        //Cleanup free tasks
        cleanup_tasks();
#ifndef  CONFIG_USE_PREEMPTION
        isix_sched_yield();
#endif
    }
}
/*-----------------------------------------------------------------------*/

#if ISIX_DEBUG_SCHEDULER == ISIX_DBG_ON

static void print_tasks(list_entry_t *sem_list)
{
   if(sem_list==NULL)
   {
        task_ready_t *i;
        task_t *j;
        isixp_sched_lock();
        printk("-------Ready tasks -------------\n");
        list_for_each_entry(&ready_task,i,inode)
        {
            printk("List inode %08x prio %d\n",(unsigned int)i,i->prio);
            list_for_each_entry(&i->task_list,j,inode)
            {
                printk("\t task %08x prio %d state %d\n",j,j->prio,j->state);
            }
        }
        printk("------ Sleeping tasks -------------\n");
        list_for_each_entry(&xxxxxx_waiting_task,j,inode)
        {
            printk("Task: %08x prio: %d state %d jiffies %d\n",j,j->prio,j->state,j->jiffies);
        }
        isixp_sched_unlock();
    }
    else
    {
       printk("------ Semaphore tasks -------------\n");
       task_t *i;
       list_for_each_entry(sem_list,i,inode_sem)
       {
         printk("Task: %08x prio: %d state %d\n",i,i->prio,i->state);
       }
    }
}


#endif

/*-----------------------------------------------------------------------*/
//Get currrent jiffies
tick_t isix_get_jiffies(void)
{
    tick_t t1,t2;
    do
    {
        t1 = jiffies;
        t2 = jiffies;
    }
    while(t1!=t2);
    return t2;
}

/*-----------------------------------------------------------------------*/
/* Initialize base OS structure before call main */
void isix_init(void)
{
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
    //Create idle task
    isix_task_create(idle_task,NULL,PORT_SCHED_MIN_STACK_DEPTH,ISIX_IDLE_PRIORITY);
}

/*-----------------------------------------------------------------------*/
/* This function start scheduler after main function */
void isix_start_scheduler(void) __attribute__((noreturn));
void isix_start_scheduler(void)
{
   isix_scheduler_running = true;
   //Restore context and run OS
   port_start_first_task();
   while(1);    //Prevent compiler warning
}

/*-----------------------------------------------------------------------*/
