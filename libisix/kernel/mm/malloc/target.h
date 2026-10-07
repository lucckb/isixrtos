#pragma once


#include "memlock.h"

#define TLSF_CREATE_LOCK(l)     do {} while(0)
#define TLSF_DESTROY_LOCK(l)    do {} while(0)
#define TLSF_ACQUIRE_LOCK(l)    mm_lock_lock()
#define TLSF_RELEASE_LOCK(l)    mm_lock_unlock()

