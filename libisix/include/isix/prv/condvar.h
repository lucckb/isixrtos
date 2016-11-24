#pragma once

#include <isix/prv/list.h>
#include <isix/prv/types.h>

struct isix_condvar {
    list_entry_t wait_list;
};


