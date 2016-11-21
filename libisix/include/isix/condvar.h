#pragma once

#include <isix/config.h>
#include <isix/types.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

struct isix_condvar;
typedef struct isix_condvar* oscondvar_t;

/** Create conditional variable
 * @return Semaphore object or NULL when semaphore can't be created
 */
oscondvar_t isix_condvar_create(void);

/** Signals one thread that is waiting on the conditional
 * @param[in] cv Conditional variable object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */

int _isixp_condvar_signal( oscondvar_t cv, bool isr );
inline __attribute__((always_inline))
	int isix_condvar_signal( oscondvar_t cv ) {
		return _isixp_condvar_signal( cv, false );
}
inline __attribute__((always_inline))
	int isix_condvar_signal_isr( oscondvar_t cv ) {
		return _isixp_condvar_signal( cv, true );
}

/** Signals all threads that are waiting on the conditional
 * @param[in] cv Conditional variable object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int _isixp_condvar_broadcast( oscondvar_t cv, bool isr );
inline __attribute__((always_inline))
	int isix_condvar_broadcast( oscondvar_t cv ) {
		return _isixp_condvar_broadcast( cv, false );
	}
inline __attribute__((always_inline))
	int isix_condvar_broadcast_isr( oscondvar_t cv ) {
		return _isixp_condvar_broadcast( cv, true );
	}




/**
 * @brief   Waits on the condition variable releasing the mutex lock.
 * @details Releases the currently owned mutex, waits on the condition
 *          variable, and finally acquires the mutex again. All the sequence
 *          is performed atomically.
 * @param[in] cv Conditional variable object
 * @param[in] timeout Max wait time
 * @post Exiting the function because a timeout does not aquire the mutex agin.
 *  The mutex ownership is lost.
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */

int isix_condvar_wait( oscondvar_t cv, ostick_t timeout );

/** Destroy the conditional variable and wakeup all waiting threads
 * @param[in] cv Conditional variable object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_condvar_destroy( oscondvar_t cv );



#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */



#ifdef __cplusplus
namespace isix {
namespace {
		inline oscondvar_t condvar_create(void) {
			return ::isix_condvar_create();
		}
		inline int condvar_signal( oscondvar_t cv ) {
			return ::isix_condvar_signal( cv );
		}
		inline int condvar_broadcast( oscondvar_t cv ) {
			return ::isix_condvar_broadcast( cv );
		}
		inline int condvar_wait( oscondvar_t cv, ostick_t timeout ) {
			return ::isix_condvar_wait( cv, timeout );
		}
		inline int condvar_destroy( oscondvar_t cv ) {
			return ::isix_condvar_destroy( cv );
		}

}}
#endif /* __cplusplus */
