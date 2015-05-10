#pragma once

#include <isix/config.h>
#include <isix/types.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

struct isix_semaphore;
typedef struct isix_semaphore* ossem_t;


/** Function create the semaphore
 * @param[in] sem Semaphore object. When it is null semaphore is created on the stack
 * @param[in] val Initial value of the semaphore
 * @param[in] limit_val Maximum value allowed in the sem
 * @return Semaphore object or NULL when semaphore can't be created
 */
ossem_t isix_sem_create_limited( ossem_t sem, int val, int limit_val );

/** Function create the semaphore
 * @param[in] sem Semaphore object. When it is null semaphore is created on the stack
 * @param[in] val Initial value of the semaphore
 * @return Semaphore object or NULL when semaphore can't be created
 */
static inline ossem_t isix_sem_create( ossem_t sem, int val )
{
	return isix_sem_create_limited( sem, val, ISIX_SEM_ULIMITED );
}
/** Wait on the semaphore P()
 * @param[in] sem Semaphore object
 * @param[in] timeout Max wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_wait( ossem_t sem, ostick_t timeout );

/** Get semaphore it also can be called from isr contest
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_trywait( ossem_t sem );

static inline __attribute__((always_inline,deprecated))
int isix_sem_get_isr( ossem_t sem )
{
	return isix_sem_trywait( sem );
}

/** Semaphore  signal V() private
 * @param[in] sem Semaphore object
 * @param[in] isr True when it is called from the ISR context
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int _isixp_sem_signal( ossem_t sem, bool isr );

/** Semaphore  signal V()
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_sem_signal( ossem_t sem )
{
	return _isixp_sem_signal( sem, false );
}

/** Semaphore  signal V() from the ISR context
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_sem_signal_isr( ossem_t sem )
{
	return _isixp_sem_signal( sem, true );
}

/** Reset semaphore to the selected value (ISR and non ISR version)
 * @param[in] sem Semaphore object
 * @param[in] val New semaphore value
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int _isixp_sem_reset( ossem_t sem, int val, bool isr );
static inline int isix_sem_reset( ossem_t sem, int val ) {
	return _isixp_sem_reset( sem, val, false );
}
static inline int isix_sem_reset_isr( ossem_t sem, int val ) {
	return _isixp_sem_reset( sem, val, true );
}

/** Get value of the semaphore
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_getval( ossem_t sem );

/** Destroy the semaphore
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_destroy( ossem_t sem );


#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */


#ifdef __cplusplus
namespace isix {
namespace {
	using sem_t = ossem_t;
	inline ossem_t sem_create_limited( ossem_t sem, int val, int limit_val ) {
		return ::isix_sem_create_limited( sem, val, limit_val );
	}
	inline ossem_t sem_create( ossem_t sem, int val ) {
		return ::isix_sem_create( sem, val );
	}
	inline int sem_wait( ossem_t sem, ostick_t timeout=ISIX_TIME_INFINITE ) {
		return ::isix_sem_wait( sem, timeout );
	}
	inline int sem_trywait( ossem_t sem ) {
		return ::isix_sem_trywait( sem );
	}
	inline int sem_signal( ossem_t sem ) {
		return ::isix_sem_signal( sem );
	}
	inline int sem_signal_isr( ossem_t sem ) {
		return ::isix_sem_signal_isr( sem );
	}
	inline int sem_getval( ossem_t sem ) {
		return ::isix_sem_getval( sem );
	}
	inline int sem_reset( ossem_t sem, int val ) {
		return ::isix_sem_reset( sem, val );
	}
	inline int sem_reset_isr( ossem_t sem, int val ) {
		return ::isix_sem_reset_isr( sem, val );
	}
	inline int sem_destroy( ossem_t sem ) {
		return ::isix_sem_destroy( sem );
	}
}}
#endif /* __cplusplus */
