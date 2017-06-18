/*
 * =====================================================================================
 *
 *       Filename:  events.h
 *
 *    Description: Isix event object type
 *
 *        Version:  1.0
 *        Created:  07.04.2015 17:35:23
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once
#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

#include <isix/prv/types.h>
#include <isix/prv/list.h>

//NOTE: Maximum number of bytes depends on the osbiset value
//! Isix event
struct isix_event {
	list_entry_t wait_list;
	osbitset_t bitset;		 //! Bit events
};

//! Special bits
#define ISIX_EVENT_CTRL_ALL_MATCH_FLAG 0x80000000U
#define ISIX_EVENT_CTRL_CLEAR_EXIT_FLAG 0x40000000U
//! Control bits
#define ISIX_EVENT_CTRL_BITS ( ISIX_EVENT_CTRL_ALL_MATCH_FLAG | \
	ISIX_EVENT_CTRL_CLEAR_EXIT_FLAG )
//! User bits
#define ISIX_EVENT_EVBITS (~ISIX_EVENT_CTRL_BITS)

