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

#include <isix/prv/types.h>
#include <isix/prv/list.h>

//! Isix event
struct isix_event {
	list_entry_t waiting_list;
	osbitset_t bitset;		 //! Bit events
	bool clear_evt_on_exit;  //! Clear evt on exit
	bool wait_for_all;   	//! Wait for all bits
};


