/*
 * multiple_objects.c
 *
 *  Created on: 24-03-2011
 *      Author: lucck
 */

/*--------------------------------------------------------------*/
#ifdef ISIX_CONFIG_USE_MULTIOBJECTS	//If timers are enabled

/*--------------------------------------------------------------*/
#include <isix/config.h>
#include <isix/types.h>
#include <isix/semaphore.h>
#include <prv/semaphore.h>
#include <prv/fifo.h>
#include <isix/multiple_objects.h>
#include <prv/list.h>
#include <prv/scheduler.h>
#include <isix/error.h>
#include <prv/multiple_objects.h>
/*--------------------------------------------------------------*/
#ifndef ISIX_DEBUG_MULTIOBJECTS
#define ISIX_DEBUG_MULTIOBJECTS ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_MULTIOBJECTS == ISIX_DBG_ON
#include <isix/printk.h>
#else
#define isix_printk(...)
#endif



/*--------------------------------------------------------------*/
//Multiple objects wating list
static list_entry_t waiting_objects;

/*--------------------------------------------------------------*/
//Initialize the multiple objects waiting
void ixixp_multiple_objects_init( void )
{
	list_init( &waiting_objects );
}

/*--------------------------------------------------------------*/
//Return taskID waiting for semgroup
int isixp_wakeup_multiple_waiting_tasks( const sem_t *sem, int (*wkup_fun)(task_t *) )
{
	isixp_enter_critical();
	prio_t prio = isix_get_min_priority();
	multiple_obj_t *mobj;
	list_for_each_entry( &waiting_objects, mobj, inode)
	{
		for(size_t s=0; s<mobj->ihwnd_len; s++)
		{
			if(mobj->ihwnd[s].sem->type == IHANDLE_T_SEM)
			{
				if( mobj->ihwnd[s].sem == sem )
				{
					if(mobj->task_id->prio < prio ) prio = mobj->task_id->prio;
					int retval = wkup_fun(mobj->task_id);
					if(retval<0) { isixp_exit_critical(); return retval;} else break;
				}
			}
			else if(mobj->ihwnd[s].sem->type == IHANDLE_T_FIFO)
			{
				if( &mobj->ihwnd[s].fifo->rx_sem == sem )
				{
					if(mobj->task_id->prio < prio ) prio = mobj->task_id->prio;
					int retval = wkup_fun(mobj->task_id);
					if(retval<0) { isixp_exit_critical(); return retval;} else break;
				}
			}
		}
	}
	isixp_exit_critical();
	return prio;
}
/*--------------------------------------------------------------*/
//Delete obj item
void isixp_delete_from_multiple_wait_list(const task_t *task )
{
	isixp_enter_critical();
	multiple_obj_t *mobj;
	list_for_each_entry( &waiting_objects, mobj, inode)
	{
		if( mobj->task_id == task )
		{
			list_delete( &mobj->inode );
			break;
		}
	}
	isixp_exit_critical();
}

/*--------------------------------------------------------------*/
int isix_wait_for_multiple_objects(size_t count, tick_t timeout, const ihandle_t *hwnd)
{
	int retval = ISIX_EOK;
	//If nothing to to - exit
	if( hwnd==NULL  ) return ISIX_EINVARG;
	multiple_obj_t multiple_obj =
	{
		hwnd, count, isix_current_task, {NULL, NULL}
	};
    //Enter critical state
	isixp_enter_critical();
	//Check if any fifo.rxsem, or sem is signaled
	for(size_t o = 0; o < count; o++)
	{
		if( hwnd[o].sem->type == IHANDLE_T_SEM )
		{
			if( hwnd[o].sem->value > 0 )
			{
				isixp_exit_critical();
                return o+1;
			}
		}
		else if( hwnd[o].sem->type == IHANDLE_T_FIFO )
		{
			if( hwnd[o].fifo->rx_sem.value > 0 )
			{
				isixp_exit_critical();
                return o+1;
			}
		}
		else
		{
			isix_bug("Obj is not fifo and it is not sem. Stack corrupted?");
		}
	}
    do {
		//Add to waiting object group
		list_insert_end( &waiting_objects, &multiple_obj.inode );
		if(isix_current_task->state & TASK_READY)
		{
				isix_current_task->state &= ~(TASK_READY| TASK_RUNNING );
				isixp_delete_task_from_ready_list(isix_current_task);
				isix_printk("Delete task from ready list");
		}
			//If any task remove task from ready list
		if(timeout)
		{
			//Add to waiting list
			isixp_add_task_to_waiting_list(isix_current_task,timeout);
			isix_current_task->state |= TASK_SLEEPING;
			isix_printk("Wait after %d ticks",isix_current_task->jiffies);
		}
		//Note state to wait for multiple objects
		isix_current_task->state |= TASK_WAITING_MULTIPLE;
		//Yield the CPU
		isixp_exit_critical();
		isix_yield();
		//Section back to the task after the wakeup
		isixp_enter_critical();
		isix_printk("After wakeup task state 0x%02x", isix_current_task->state );
		if(!(isix_current_task->state & TASK_MULTIPLE_WKUP))
		{
			retval = ISIX_ETIMEOUT;
			break;
		}
		for( size_t o = 0; o < count; o++ )
		{
			if( hwnd[o].sem->type == IHANDLE_T_SEM )
			{
				isix_printk("Sem value %d",hwnd[o].sem->value);
				if( hwnd[o].sem->value > 0)
				{
					retval = o+1; break;
				}
			}
			else if( hwnd[o].sem->type == IHANDLE_T_FIFO )
			{
				isix_printk("Fifo value %d",hwnd[o].sem->value);
				if( hwnd[o].fifo->rx_sem.value > 0 )
				{
					retval = o+1; break;
				}
			}
			else
			{
				isix_bug("Obj is not fifo and it is not sem. Stack corrupted?");
			}
		}
	} while(0);
	//Delete from object group list
	if( multiple_obj.inode.next ) list_delete( &multiple_obj.inode );
	if( retval == ISIX_EOK )
	{
		isix_bug("Waiting task not found when it is excepted");
	}
	//Exit critical section
	isixp_exit_critical();
	//return
	return retval;
}
/*--------------------------------------------------------------*/
#else

//NOTE: avoid warning iso C forbid empty translation unit
static inline void ixixp_multiple_objects_init( void ) {}

#endif /* ISIX_CONFIG_USE_MULTIOBJECTS */
