/*
 * =====================================================================================
 *
 *       Filename:  workqueue.h
 *
 *    Description:  Workqueue implementation 
 *
 *        Version:  1.0
 *        Created:  13.04.2015 17:16:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once

#include <isix/types.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

struct isix_workqueue;
//! Workqueue function
typedef struct isix_workqueue osworkqueue_t;
//! Oswork queue function callback
typedef void(*osworkqueue_func_t)(void*);


inline int isix_schedule_delayed_work( osworkqueue_t* func, 
		void* arg, ostick_t timeout );

int isix_schedule_work( osworkqueue_t* func, void* arg );

int isix_cancel_dalayed_work(

#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */

