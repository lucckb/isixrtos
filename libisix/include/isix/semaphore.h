#pragma once

#include <isix/config.h>
#include <isix/types.h>

/*--------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
/*--------------------------------------------------------------*/

struct isix_semaphore;
typedef struct isix_semaphore* ossem_t;

/*--------------------------------------------------------------*/
#ifdef __cplusplus
static const unsigned ISIX_TIME_INFINITE = 0;
static const unsigned ISIX_SEM_ULIMITED = 0;
#else
#define ISIX_TIME_INFINITE (0)
#define ISIX_SEM_ULIMITED (0)
#endif /*__cplusplus*/
/*--------------------------------------------------------------*/
/** Function create the semaphore
 * @param[in] sem Semaphore object. When it is null semaphore is created on the stack
 * @param[in] val Initial value of the semaphore
 * @param[in] limit_val Maximum value allowed in the sem
 * @return Semaphore object or NULL when semaphore can't be created
 */
ossem_t isix_sem_create_limited( ossem_t sem, int val, int limit_val);

/*--------------------------------------------------------------*/
/** Function create the semaphore
 * @param[in] sem Semaphore object. When it is null semaphore is created on the stack
 * @param[in] val Initial value of the semaphore
 * @return Semaphore object or NULL when semaphore can't be created
 */
static inline ossem_t isix_sem_create( ossem_t sem,int val)
{
	return isix_sem_create_limited( sem, val, ISIX_SEM_ULIMITED );
}
/*--------------------------------------------------------------*/
/** Wait on the semaphore P()
 * @param[in] sem Semaphore object
 * @param[in] timeout Max wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_wait( ossem_t sem, ostick_t timeout );

/*--------------------------------------------------------------*/
/** Get semaphore from the ISR context
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_get_isr( ossem_t sem );

/*--------------------------------------------------------------*/
/** Semaphore  signal V() private
 * @param[in] sem Semaphore object
 * @param[in] isr True when it is called from the ISR context
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int _isixp_sem_signal( ossem_t sem, bool isr );

/*--------------------------------------------------------------*/
/** Semaphore  signal V()
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_sem_signal( ossem_t sem )
{
	return _isixp_sem_signal( sem, false );
}

/*--------------------------------------------------------------*/
/** Semaphore  signal V() from the ISR context
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_sem_signal_isr( ossem_t sem )
{
	return _isixp_sem_signal(sem,true);
}

/*--------------------------------------------------------------*/
/** Set value of the semaphore
 * @param[in] sem Semaphore object
 * @param[in] val New semaphore value
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_setval( ossem_t sem,int val );

/*--------------------------------------------------------------*/
/** Get value of the semaphore
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_getval( ossem_t sem );

/*--------------------------------------------------------------*/
/** Destroy the semaphore
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_destroy( ossem_t sem );

/*--------------------------------------------------------------*/
/** Convert ms value to the system tick value
 * @param[in] ms Time value in the millisecond
 * @return Sys tick time value
 */
ostick_t isix_ms2tick( unsigned long ms );

/*--------------------------------------------------------------*/

/** Wait thread for selected number of ticks
 * @param[in] timeout Wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_wait( ostick_t timeout );

/*-----------------------------------------------------------*/
/** Wait thread for selected number of milliseconds
 * @param[in] timeout Wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_wait_ms( unsigned long ms )
{
	return isix_wait(isix_ms2tick(ms));
}

/*-----------------------------------------------------------*/

#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */

