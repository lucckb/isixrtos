#pragma once

#include <isix/prv/list.h>
#include <isix/prv/types.h>
#include <stdatomic.h>

struct isix_task;

// Structure of isix mutex
struct isix_mutex {
	// Numer of locked count
	atomic_int count;
	// Mutex owner
	struct isix_task* owner;
	// Numer of tasks waiting on mutex
	list_entry_t wait_list;
	// Static alocated mutex
	bool static_mem;
};
