/*
 * =====================================================================================
 *
 *       Filename:  seqfit_config.h
 *
 *    Description:  SEQ fit memory allocation config
 *
 *        Version:  1.0
 *        Created:  18.06.2017 19:39:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once


#include "memlock.h"

/* USE SEQFIT locking */
#define SEQFIT_LOCK (1)

#define SEQFIT_MLOCK_T            struct isix_mutex
#define SEQFIT_CREATE_LOCK(l)     mm_lock_init(l)
#define SEQFIT_ACQUIRE_LOCK(l)    mm_lock_lock(l)
#define SEQFIT_RELEASE_LOCK(l)    mm_lock_unlock(l)

