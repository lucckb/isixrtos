/*
 * =====================================================================================
 *
 *       Filename:  diag.c
 *
 *    Description:  ISIX diagnostics function
 *
 *        Version:  1.0
 *        Created:  23 lut 2008 18:32:01
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza <lucjan.bryndza@ep.com.pl>
 *        Company:  Boff
 *
 * =====================================================================================
 */

#include <isix/types.h>
#include <prv/scheduler.h>
#include <prv/semaphore.h>
#include <isix/printk.h>
#include <asm/interrupt.h>


/*-------------------------------------------------*/
/* Display and print bug */
void bug(void)
{
    //Lock scheduler forever...
    sched_lock();
    //Print error
    printk("OOPS - #Please reset board# \n");
    //Disable interrupts
    fiqirq_disable();
    //Get ready tasks
    const list_entry_t *entry = get_scheduler_state(TASK_READY);
    task_ready_t *i;
    task_t *j;
    sem_t *s;
    sched_lock();
    printk("Ready tasks:\n");
    list_for_each_entry(entry,i,inode)
    {
        printk("\t* Priority: %08x prio: %d\n",(unsigned int)i,i->prio);
        list_for_each_entry(&i->task_list,j,inode)
        {
            printk("\t\t-> task: %08x prio: %d state: %d\n",j,j->prio,j->state);
        }
    }
    entry = get_scheduler_state(TASK_SLEEPING);
    printk("Sleeping tasks:\n");
    list_for_each_entry(entry,j,inode)
    {
          printk("\t* Task: %08x prio: %d state %d jiffies %d\n",j,j->prio,j->state,j->jiffies);
    }
    printk("Semaphore list:\n");
    entry = get_sem_list();
    list_for_each_entry(entry,s,inode)
    {
       printk("\t* Sem: %08x Val: %d IntMask %08x\n",s,s->value,s->intmask);
       list_for_each_entry(&s->sem_task,j,inode)
       {
            printk("\t\t-> task: %08x prio: %d state: %d\n",j,j->prio,j->state);
       }
    }
    //Loop forever
    while(1);
}


