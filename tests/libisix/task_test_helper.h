/*
 * =====================================================================================
 *
 *       Filename:  task_test_helper.h
 *
 *    Description:  Task test helper hacker
 *
 *        Version:  1.0
 *        Created:  24.11.2016 19:00:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

size_t thack_struct_size(void);
int thack_getref_cnt( struct isix_task* t );
int thack_task_wait_list_is_empty( struct isix_task* t );

#ifdef __cplusplus
}
#endif


