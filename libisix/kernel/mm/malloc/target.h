#pragma once


#include "memlock.h"

#define TLSF_MLOCK_T            struct isix_mutex
#define TLSF_CREATE_LOCK(l)     mm_lock_init(l)
#define TLSF_DESTROY_LOCK(l)    do {} while(0)
#define TLSF_ACQUIRE_LOCK(l)    mm_lock_lock(l)
#define TLSF_RELEASE_LOCK(l)    mm_lock_unlock(l)

