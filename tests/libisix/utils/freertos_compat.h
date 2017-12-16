/*
 * =====================================================================================
 *
 *       Filename:  freertos_compat.h
 *
 *    Description:  Freertos compatibility layer
 *
 *        Version:  1.0
 *        Created:  10.04.2015 17:15:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once
#include <isix.h>
#include <foundation/sys/dbglog.h>

typedef int BaseType_t;
typedef ostask_t TaskHandle_t;
typedef osevent_t EventGroupHandle_t;
typedef unsigned EventBits_t;
typedef unsigned TickType_t;

static inline void xTaskCreate( void (*fun)(void*), const char* name,
		unsigned stack_size, void*param, osprio_t priority, 
		TaskHandle_t* task ) 
{
	ostask_t tid = isix_task_create(fun,param,stack_size,priority, 0 );
	if( task ) *task = tid;
	(void)name;
}

#define configMINIMAL_STACK_SIZE 512
#define tskIDLE_PRIORITY 3

#define configASSERT(x) do { if(!(x)) { dbprintf("Assertion failed %s in %s:%i", #x, __FILE__, __LINE__ ); isix_bug("assert");  }} while(0)



#define pdTRUE true
#define pdFALSE false
#define portMAX_DELAY ISIX_TIME_INFINITE
#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )

static inline EventGroupHandle_t xEventGroupCreate( void )
{
	return isix_event_create();
}

static inline void vEventGroupDelete( EventGroupHandle_t xEventGroup )
{
	isix_event_destroy( xEventGroup );
}

static inline EventBits_t xEventGroupSync( EventGroupHandle_t xEventGroup, 
		const EventBits_t uxBitsToSet, const EventBits_t uxBitsToWaitFor, 
		TickType_t xTicksToWait ) 
{
	osbitset_ret_t r = isix_event_sync( xEventGroup, uxBitsToSet, uxBitsToWaitFor, xTicksToWait );
	if( r < 0 ) {
		r = 0;
	}
	return r;
}

static inline EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup, 
		const EventBits_t uxBitsToWaitFor, const bool xClearOnExit, 
		const bool xWaitForAllBits, TickType_t xTicksToWait )
{
	osbitset_ret_t r = isix_event_wait( xEventGroup, uxBitsToWaitFor, 
				   xClearOnExit, xWaitForAllBits, xTicksToWait );
	if( r < 0 ) r = 0;
	return r;
}

static inline EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup, 
		const EventBits_t uxBitsToSet )
{
	osbitset_ret_t r = isix_event_set( xEventGroup, uxBitsToSet );
	if( r < 0 ) r = 0;
	return r;
}

#define xEventGroupGetBits( xEventGroup ) xEventGroupClearBits( xEventGroup, 0 )

static inline EventBits_t xEventGroupClearBits( EventGroupHandle_t xEventGroup, 
		const EventBits_t uxBitsToClear )
{

	osbitset_ret_t r = isix_event_clear( xEventGroup, uxBitsToClear );
	if( r < 0 ) r = 0;
	return r;
}

static inline BaseType_t xEventGroupClearBitsFromISR( EventGroupHandle_t xEventGroup, 
		const EventBits_t uxBitsToSet )
{
	osbitset_ret_t r = isix_event_clear_isr( xEventGroup, uxBitsToSet );
	if( r < 0 ) r = 0;
	return r;
}

static inline EventBits_t xEventGroupGetBitsFromISR( EventGroupHandle_t xEventGroup )
{
	osbitset_ret_t r = isix_event_get_isr( xEventGroup );
	if( r < 0 ) r = 0;
	return r;
}


static inline BaseType_t xEventGroupSetBitsFromISR( EventGroupHandle_t xEventGroup, 
		const EventBits_t uxBitsToSet, BaseType_t *pxHigherPriorityTaskWoken )
{
	(void)pxHigherPriorityTaskWoken ;
	osbitset_ret_t r = isix_event_set_isr(xEventGroup, uxBitsToSet );
	if( r < 0 ) r = 0;
	return r;
}

#define eTaskGetState isix_get_task_state
#define vTaskSuspend isix_task_suspend
#define eSuspended  OSTHR_STATE_SUSPEND
#define eBlocked  OSTHR_STATE_WTEVT
#define eReady OSTHR_STATE_READY
#define vTaskResume isix_task_resume
#define vTaskPrioritySet isix_task_change_prio
#define xTaskGetCurrentTaskHandle isix_task_self


