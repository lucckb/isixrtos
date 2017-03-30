/*
 * =====================================================================================
 *
 *       Filename:  task_test_helper.c
 *
 *    Description:  Task hack test helper
 *
 *        Version:  1.0
 *        Created:  24.11.2016 18:56:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
#include <isix/prv/list.h>

size_t thack_struct_size(void) {
	//8 is malloc list hdr + align
	return sizeof( struct isix_task ) + 8 + 4;
}

int thack_getref_cnt( struct isix_task* t ) {
	return  t->refcnt;
}

int thack_task_wait_list_is_empty( struct isix_task* t ) {
	return  list_isempty( &t->waiting_tasks );
}



