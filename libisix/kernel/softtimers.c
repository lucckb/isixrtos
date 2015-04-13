/*
 * irqtimers.c
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#include <isix/prv/softtimers.h>
#include <isix/softtimers.h>
#include <isix/memory.h>
#include <isix/prv/list.h>
#include <string.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>

#ifdef ISIX_CONFIG_USE_TIMERS 

static struct vtimer_context tctx;

//Initialize vtimers infrastructure
void _isixp_vtimer_init(void)
{
	list_init( &tctx.vtimer_list[0] );
	list_init( &tctx.vtimer_list[1] );
	//Initialize overflow waiting list
	tctx.p_vtimer_list = &tctx.vtimer_list[0];
	tctx.pov_vtimer_list = &tctx.vtimer_list[1];
}


//Move selected task to waiting list
static void add_vtimer_to_list(osvtimer_t timer)
{
    //Scheduler lock
    _isixp_enter_critical();
    timer->jiffies = isix_get_jiffies() + timer->timeout;
    if(timer->jiffies < isix_get_jiffies())
    {
    	//Insert on overflow waiting list in time order
    	osvtimer_t waitl;
    	list_for_each_entry(tctx.pov_vtimer_list,waitl,inode)
    	{
    	   if( timer->jiffies < waitl->jiffies ) break;
    	}
    	list_insert_before(&waitl->inode,&timer->inode);
    }
    else
    {
    	//Insert on waiting list in time order no overflow
    	osvtimer_t waitl;
    	list_for_each_entry( tctx.p_vtimer_list, waitl, inode )
    	{
    	    if( timer->jiffies < waitl->jiffies ) break;
    	}
    	list_insert_before( &waitl->inode, &timer->inode );
    }
    //Scheduler unlock
    _isixp_exit_critical();
}


//Call timer funcs in the interrupt context
void _isixp_vtimer_handle_time(ostick_t jiffies)
{
	if(jiffies == 0)
	{
	   list_entry_t *tmp = tctx.p_vtimer_list;
	   tctx.p_vtimer_list = tctx.pov_vtimer_list;
	   tctx.pov_vtimer_list = tmp;
	}
	osvtimer_t vtimer;
	while( !list_isempty(tctx.p_vtimer_list) &&
		jiffies>=(vtimer = list_first_entry(tctx.p_vtimer_list,inode,struct isix_vtimer))->jiffies
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


//Create the virtual timer
osvtimer_t _isix_vtimer_create_internal_(osvtimer_callback func,void *arg, bool one_shoot )
{
	osvtimer_t timer = (osvtimer_t)isix_alloc(sizeof(struct isix_vtimer));
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


//Start the virtual timer
int isix_vtimer_start(osvtimer_t timer, ostick_t timeout)
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

//! Start one shoot timer
int isix_vtimer_one_shoot( osvtimer_t timer, osvtimer_callback func, void *arg, ostick_t timeout )
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

//Destroy the virtual timer
int isix_vtimer_destroy(osvtimer_t timer)
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


#endif /* ISIX_CONFIG_USE_TIMERS */

