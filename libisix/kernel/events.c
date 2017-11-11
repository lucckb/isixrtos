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
#include <isix/prv/printk.h>
#include <isix/prv/scheduler.h>
#include <isix/prv/events.h>
#include <isix/events.h>
#include <isix/memory.h>
#include <isix/task.h>
#include <string.h>

#ifdef CONFIG_ISIX_LOGLEVEL_EVENTS
#undef CONFIG_ISIX_LOGLEVEL 
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_EVENTS
#endif
#include <isix/prv/printk.h>

//! Check waiting condition
static inline bool check_cond( osbitset_t curr, osbitset_t wait_for, bool all )
{
	bool cond;
	if( !all ) {
		cond = (curr & wait_for) != 0U;
	} else {
		cond = (curr & wait_for)==wait_for;
	}
	return cond;
}
static inline bool check_cond2( osbitset_t curr, osbitset_t wait_for )
{
	return check_cond( curr, wait_for&ISIX_EVENT_EVBITS,
			wait_for & ISIX_EVENT_CTRL_ALL_MATCH_FLAG );
}

//!Create event
osevent_t isix_event_create( void )
{
	osevent_t ev = isix_alloc( sizeof( struct isix_event ) );
	if( !ev ) {
		pr_err("Unable to create events");
		return ev;
	}
	memset( ev, 0, sizeof(*ev) );
	list_init( &ev->wait_list );
	return ev;
}

//! Delete an event
//! Wakekup all and next delete it
int isix_event_destroy( osevent_t evh )
{
	if( !evh ) {
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	ostask_t wkup_task = currp;
	ostask_t t;
	while( (t=_isixp_remove_from_prio_queue(&evh->wait_list) ) )
	{	
		_isixp_wakeup_task_l( t, ISIX_EDESTROY );
		 wkup_task = _isixp_max_prio( wkup_task, t );
	}
	_isixp_do_reschedule( wkup_task );
	isix_free( evh );
	return ISIX_EOK;
}

// Wait for an event
osbitset_ret_t isix_event_wait( osevent_t evth, osbitset_t bits_to_wait, 
		bool clear_on_exit, bool wait_for_all, ostick_t timeout )
{
	osbitset_t retval;
	//! Check input parameters
	if( !evth ) {
		return ISIX_EINVARG;
	}
	if( !bits_to_wait || (bits_to_wait&ISIX_EVENT_CTRL_BITS) ) {
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	if( check_cond(evth->bitset, bits_to_wait, wait_for_all) )
	{
		retval = evth->bitset;
		//! Condition was meet so not need to wait
		if( clear_on_exit ) {
			evth->bitset &= ~bits_to_wait;
		}
	}
	else if( timeout == ISIX_TIME_DONTWAIT )
	{
		//! Don't wait only return current bitmask
		retval = evth->bitset;
	}
	else
	{
		//! Condition as not meet so need to wait for data
		_isixp_set_sleep_timeout( OSTHR_STATE_WTEVT, timeout );	//Goto sleep
		list_insert_end( &evth->wait_list, &currp->inode );	//Place on bitset list
		currp->obj.evbits = bits_to_wait |
			( wait_for_all?ISIX_EVENT_CTRL_ALL_MATCH_FLAG:0U ) |
			( clear_on_exit?ISIX_EVENT_CTRL_CLEAR_EXIT_FLAG:0U ) ;
		isix_exit_critical();
		isix_yield();
		isix_enter_critical();
		if( (currp->obj.evbits&ISIX_EVENT_CTRL_BITS)==0 )
		{	//! Wakeup all bits should be set
			retval = currp->obj.evbits;
		} else {
			if( clear_on_exit ) {
				if( check_cond(evth->bitset, bits_to_wait, wait_for_all) ) {
					evth->bitset &= ~bits_to_wait;
				}
			}
			//!31th bit set it is timeout
			retval = currp->obj.dmsg;
		}
	}
	isix_exit_critical();
	return retval;
}

//! Event clear bits
osbitset_ret_t isix_event_clear( osevent_t evth, osbitset_t bits_to_clear )
{
	osbitset_t retval;
	//! Check input parameters
	if( !evth ) {
		return ISIX_EINVARG;
	}
	if( bits_to_clear&ISIX_EVENT_CTRL_BITS ) {
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	retval = evth->bitset;
	evth->bitset &= ~bits_to_clear;
	isix_exit_critical();
	return retval;
}


static ostask_t event_set( osevent_t evth, osbitset_t bits_to_set )
{
	evth->bitset |= bits_to_set;
	ostask_t wkup_task = currp;
	osbitset_t clr_bits = 0U;
	ostask_t t, tmp;
	list_for_each_entry_safe( &evth->wait_list, t, tmp, inode )
	{
		if( check_cond2(evth->bitset,t->obj.evbits) )
		{
			if( t->obj.evbits & ISIX_EVENT_CTRL_CLEAR_EXIT_FLAG ) {
				clr_bits |= t->obj.evbits & ISIX_EVENT_EVBITS;
			}
			//! Post only normal bits negative value means error
			list_delete( &t->inode );
			_isixp_wakeup_task_l( t, evth->bitset );
			wkup_task = _isixp_max_prio( wkup_task, t );
		}
	}
	evth->bitset &= ~clr_bits;
	return wkup_task;
}


//! Events to set bits
osbitset_ret_t _isixp_event_set( osevent_t evth, osbitset_t bits_to_set, bool isr )
{
	//! Check input parameters
	if( !evth ) {
		return ISIX_EINVARG;
	}
	if( bits_to_set&ISIX_EVENT_CTRL_BITS ) {
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	ostask_t wkup_task =  event_set( evth, bits_to_set );
	if( !isr ) _isixp_do_reschedule( wkup_task );
	else	   isix_exit_critical();
	return evth->bitset;
}

//! Get bits from the ISR
osbitset_ret_t isix_event_get_isr( osevent_t evth )
{
	if( !evth ) {
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	osbitset_t retval = evth->bitset;
	isix_exit_critical();
	return retval;
}

//Atomically sets bits than wait
osbitset_ret_t isix_event_sync( osevent_t evth, osbitset_t bits_to_set,
	osbitset_t bits_to_wait, ostick_t timeout )
{
	osbitset_t retval;
	if( !evth ) {
		return ISIX_EINVARG;
	}
	if( bits_to_set&ISIX_EVENT_CTRL_BITS ) {
		return ISIX_EINVARG;
	}
	if( !bits_to_wait||(bits_to_wait&ISIX_EVENT_CTRL_BITS) ) {
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	osbitset_t orgbits = evth->bitset;
	ostask_t wkup_task = event_set( evth, bits_to_set );
	if( ((orgbits|bits_to_set) & bits_to_wait) == bits_to_wait )
	{
		//! All bits set condition was meet
		retval = orgbits|bits_to_set;
		evth->bitset &= ~ bits_to_wait;
	}
	else
	{
		//Bit condition not meet
		if( timeout != ISIX_TIME_DONTWAIT )
		{
			_isixp_set_sleep_timeout( OSTHR_STATE_WTEVT, timeout );	//Goto sleep
			list_insert_end( &evth->wait_list, &currp->inode );	//Place on bitset list
			currp->obj.evbits =  bits_to_wait| ISIX_EVENT_CTRL_ALL_MATCH_FLAG
								| ISIX_EVENT_CTRL_CLEAR_EXIT_FLAG;
			if( wkup_task!= currp ) _isixp_do_reschedule( wkup_task );
			else { isix_exit_critical(); isix_yield(); }
			wkup_task = currp;
			isix_enter_critical();
			if( (currp->obj.evbits&ISIX_EVENT_CTRL_BITS) == 0 ) {
				retval = currp->obj.evbits;
			} else {
				if( (evth->bitset&bits_to_wait) == bits_to_wait ) {
					evth->bitset &= ~bits_to_wait;
				}
				//!31th bit set it is timeout
				retval = currp->obj.dmsg;
			}
		}
		else
		{
			retval = evth->bitset;
		}
	}
	_isixp_do_reschedule( wkup_task );
	return retval;
}
