#pragma once
#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

#include <isix/prv/list.h>
#include <isix/prv/types.h>

struct isix_condvar {
    list_entry_t wait_list;
};


