/*
 * multiple_objects.h
 *
 *  Created on: 24-03-2011
 *      Author: lucck
 */

#ifndef ISIX_MULTIPLE_OBJECTS_H_
#define ISIX_MULTIPLE_OBJECTS_H_
/*--------------------------------------------------------------*/
#ifdef ISIX_CONFIG_USE_MULTIOBJECTS

/*--------------------------------------------------------------*/
#include <isix/types.h>
#include <isix/semaphore.h>
#include <isix/fifo.h>
#include <isix/ihandle.h>
/*--------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*--------------------------------------------------------------*/
/**
 * Waits until any of objects in array is signaled
 * (sem is signaled or fifo has data in queue.) Function doesn't wait
 * for no space in fifo, so the fifo can block, when user tried to push data
 * into fifo when space in fifo is not available. After call this function user
 * must call sem_wait, or fifo_get , on object which was signalled.
 * @param[in] count Number of objects for wait
 * @param[in] timeout Timeout in ticks, or @see ISIX_TIME_INFINITE for infinite waiting
 * @param[in] hwnd Array of objects for waiting.
 * @return Function return error code if value is <0 for example
 * @see ISIX_ETIMEOUT when timeout reache and any objects is not signalled.
 * On positive value, return an object index which was signaled.
 * For example if 1 is returned it means that first object from hwnd, array was signalled.
 */
int isix_wait_for_multiple_objects(size_t count, tick_t timeout, const ihandle_t *hwnd);

/*--------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */
/*--------------------------------------------------------------*/
#endif  /*ISIX_CONFIG_USE_MULTIOBJECTS */
/*--------------------------------------------------------------*/
#endif /* MULTIPLE_OBJECTS_H_ */
