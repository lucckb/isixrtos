/*
 * irqtimers.c
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */
/*-----------------------------------------------------------------------*/
#include <isix/prv/irqtimers.h>
#include <isix/irqtimers.h>
#include <isix/memory.h>
#include <isix/prv/list.h>
#include <isix/prv/scheduler.h>
#include <string.h>

/*-----------------------------------------------------------------------*/
#ifdef ISIX_CONFIG_USE_TIMERS 
/*-----------------------------------------------------------------------*/
//List entry for the virtual timers
static list_entry_t vtimer_list[2];
//Overflowed and not overflowed list
static list_entry_t *p_vtimer_list;
static list_entry_t *pov_vtimer_list;
/*-----------------------------------------------------------------------*/
//Initialize vtimers infrastructure
void _isixp_vtimer_init(void)
{
	list_init( &vtimer_list[0] );
	list_init( &vtimer_list[1] );
	//Initialize overflow waiting list
	p_vtimer_list = &vtimer_list[0];
	pov_vtimer_list = &vtimer_list[1];
}

/*-----------------------------------------------------------------------*/
//Move selected task to waiting list
static void add_vtimer_to_list(vtimer_t *timer)
{
    //Scheduler lock
    _isixp_enter_critical();
    timer->jiffies = isix_get_jiffies() + timer->timeout;
    if(timer->jiffies < isix_get_jiffies())
    {
    	//Insert on overflow waiting list in time order
    	vtimer_t *waitl;
    	list_for_each_entry(pov_vtimer_list,waitl,inode)
    	{
    	   if(timer->jiffies<waitl->jiffies) break;
    	}
    	list_insert_before(&waitl->inode,&timer->inode);
    }
    else
    {
    	//Insert on waiting list in time order no overflow
    	vtimer_t *waitl;
    	list_for_each_entry(p_vtimer_list,waitl,inode)
    	{
    	    if(timer->jiffies<waitl->jiffies) break;
    	}
    	list_insert_before(&waitl->inode,&timer->inode);
    }
    //Scheduler unlock
    _isixp_exit_critical();
}

/*-----------------------------------------------------------------------*/
//Call timer funcs in the interrupt context
void _isixp_vtimer_handle_time(tick_t jiffies)
{
	if(jiffies == 0)
	{
	   list_entry_t *tmp = p_vtimer_list;
	   p_vtimer_list = pov_vtimer_list;
	   pov_vtimer_list = tmp;
	}
	vtimer_t *vtimer;
	while( !list_isempty(p_vtimer_list) &&
	    		jiffies>=(vtimer = list_get_first(p_vtimer_list,inode,vtimer_t))->jiffies
	      )
	{
		list_delete(&vtimer->inode);
		if( vtimer->timer_handler ) 
			vtimer->timer_handler( vtimer->arg );
		if( !vtimer->one_shoot ) 
		{
			add_vtimer_to_list(vtimer);
		}
	}
}

/*-----------------------------------------------------------------------*/
//Create the virtual timer
vtimer_t* _isix_vtimer_create_internal_(void (*func)(void*),void *arg, bool one_shoot )
{
	vtimer_t * const timer = (vtimer_t*)isix_alloc(sizeof(vtimer_t));
	if( timer == NULL ) {
		return NULL;
	}
	if( !one_shoot ) {
		if( func == NULL ) {
			isix_free( timer );
			return NULL;
		}
	} else {
		if( func ) {
			isix_free( timer );
			return NULL;
		}
	}
    memset( timer, 0, sizeof(*timer) );
    timer->arg = arg;
    timer->timer_handler = func;
    timer->one_shoot = one_shoot;
    return timer;
}

/*-----------------------------------------------------------------------*/
//Start the virtual timer
int isix_vtimer_start(vtimer_t* timer, tick_t timeout)
{
	if( timer == NULL ) return ISIX_EINVARG;
	if( timer->one_shoot && timeout > 0 ) return ISIX_EINVARG;
	_isixp_enter_critical();
	//Search on ov list
	if( list_is_elem_assigned( &timer->inode ) )
	{
		list_delete( &timer->inode );
	}
	//Add timer to waiting list
	if( timeout > 0 )
	{
		timer->timeout = timeout;
		add_vtimer_to_list( timer );
	}
	_isixp_exit_critical();
	return ISIX_EOK;
}
/*-----------------------------------------------------------------------*/
//! Start one shoot timer
int isix_vtimer_one_shoot( vtimer_t* timer, void (*func)(void*), void *arg, tick_t timeout )
{
	if( timer == NULL ) return ISIX_EINVARG;
	if( !timer->one_shoot ) return ISIX_EINVARG;
	_isixp_enter_critical();
	if( list_is_elem_assigned( &timer->inode ) )
	{
		_isixp_exit_critical();
		return ISIX_EBUSY;
	}
	if( timeout > 0 && timer )
	{
		timer->timeout = timeout;
		timer->timer_handler = func;
		timer->arg = arg;
		add_vtimer_to_list( timer );
	}
	_isixp_exit_critical();
	return ISIX_EOK;
}
/*-----------------------------------------------------------------------*/
//Destroy the virtual timer
int isix_vtimer_destroy(vtimer_t* timer)
{
	if( timer == NULL ) return ISIX_EINVARG;
	_isixp_enter_critical();
	if( list_is_elem_assigned( &timer->inode ) )
	{
		_isixp_exit_critical();
		return ISIX_EBUSY;
	}
	isix_free( timer );
	_isixp_exit_critical();
	return ISIX_EOK;
}
/*-----------------------------------------------------------------------*/


#endif /* ISIX_CONFIG_USE_TIMERS */

