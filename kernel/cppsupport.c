/* ------------------------------------------------------------------ */
/*
 * cppsupport.c
 *
 *  Created on: 2009-12-13
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <isix.h>
#include <isix/printk.h>
#include <prv/scheduler.h>
/* ------------------------------------------------------------------ */
static sem_t *ctors_sem;

/* ------------------------------------------------------------------ */
/* thread safe constructed objects  */
void __cxa_guard_acquire ()
{
  if(ctors_sem==NULL)
  {
	  ctors_sem = isix_sem_create(NULL,1);
	  if(ctors_sem==NULL)
	  {
		  isix_printk("FATAL: Cannot allocate CPP guard semaphore");
		  //Remove task if can;t create semaphore
		  isix_task_delete(NULL);
	  }
  }
  if(ctors_sem && isix_scheduler_running)
  {
	  isix_sem_wait(ctors_sem,ISIX_TIME_INFINITE);
  }
}

/* ------------------------------------------------------------------ */
void __cxa_guard_release ()
{
	if(ctors_sem && isix_scheduler_running)
	{
		isix_sem_signal(ctors_sem);
	}
}

/* ------------------------------------------------------------------ */
//Pure virtual call error handler
void __cxa_pure_virtual()
{
	isix_printk("FATAL: Pure virtual function called");
	//Remove task if can;t create semaphore
	isix_task_delete(NULL);
}

/* ------------------------------------------------------------------ */
