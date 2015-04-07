/*
 * =====================================================================================
 *
 *       Filename:  events.c
 *
 *    Description:  ISIX rtos events added
 *
 *        Version:  1.0
 *        Created:  07.04.2015 13:08:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */
#include <isix/printk.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
#include <isix/prv/events.h>
#include <isix/events.h>
#include <isix/memory.h>
#include <isix/task.h>
#include <string.h>

/* ------------------------------------------------------------------ */ 
//!Create event
osevent_t isix_event_create( void )
{
	osevent_t ev = isix_alloc( sizeof( struct isix_event ) );
	if( !ev ) {
		printk("Unable to create events");
		return ev;
	}
	memset( ev, 0, sizeof(*ev) );
	list_init( &ev->waiting_list );
	return ev;
}
/* ------------------------------------------------------------------ */ 
//! Delete an event
//! Wakekup all and next delete it
int isix_event_delete( osevent_t evh )
{
	if( !evh ) {
		return ISIX_EINVARG;
	}
	_isixp_enter_critical();
	ostask_t wkup_task = currp;
	for( ostask_t t = _isixp_remove_from_prio_queue(&evh->waiting_list);t;)
	{	
		_isixp_wakeup_task_l( t, ISIX_ERESET );
		 wkup_task = isixp_max_prio( wkup_task, t );
	}
	_isixp_do_reschedule( wkup_task );
	return ISIX_EOK;
}
/* ------------------------------------------------------------------ */ 
