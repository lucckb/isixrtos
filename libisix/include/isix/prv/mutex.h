#pragma once

#include <isix/prv/list.h>
#include <isix/prv/types.h>

struct isix_task;

// Structure of isix mutex
struct isix_mutex {
	// Numer of locked count
	int count;
	// Mutex owner
	struct isix_task* owner;
	//Task owner list
	list_t inode;
	// Numer of tasks waiting on mutex
	list_entry_t wait_list;
	// Static alocated mutex
	bool static_mem;
};


struct isix_task;
//! Unlock all thread waiting on the selected task
void _isixp_mutex_unlock_all_in_task( struct isix_task* utask );

//! Get first Mutex owner and release it
struct isix_mutex*  _isixp_get_top_currt_mutex( void );
