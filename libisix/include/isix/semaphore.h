#pragma once

#include <isix/config.h>
#include <isix/types.h>

/*--------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/
/*--------------------------------------------------------------*/

struct sem_struct;
typedef struct sem_struct sem_t;
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
sem_t* isix_sem_create_limited(sem_t *sem,int val, int limit_val);

/*--------------------------------------------------------------*/
/** Function create the semaphore
 * @param[in] sem Semaphore object. When it is null semaphore is created on the stack
 * @param[in] val Initial value of the semaphore
 * @return Semaphore object or NULL when semaphore can't be created
 */
static inline sem_t* isix_sem_create(sem_t *sem,int val)
{
	return isix_sem_create_limited( sem, val, ISIX_SEM_ULIMITED );
}

/*--------------------------------------------------------------*/
/** Wait on the semaphore P()
 * @param[in] sem Semaphore object
 * @param[in] timeout Max wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_wait(sem_t *sem, tick_t timeout);

/*--------------------------------------------------------------*/
/** Get semaphore from the ISR context
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_get_isr(sem_t *sem);

/*--------------------------------------------------------------*/
/** Semaphore  signal V() private
 * @param[in] sem Semaphore object
 * @param[in] isr True when it is called from the ISR context
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int _isixp_sem_signal(sem_t *sem, bool isr);

/*--------------------------------------------------------------*/
/** Semaphore  signal V()
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_sem_signal(sem_t *sem)
{
	return _isixp_sem_signal( sem, false );
}

/*--------------------------------------------------------------*/
/** Semaphore  signal V() from the ISR context
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_sem_signal_isr(sem_t *sem)
{
	return _isixp_sem_signal(sem,true);
}

/*--------------------------------------------------------------*/
/** Set value of the semaphore
 * @param[in] sem Semaphore object
 * @param[in] val New semaphore value
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_setval(sem_t *sem,int val);

/*--------------------------------------------------------------*/
/** Get value of the semaphore
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_getval(sem_t *sem);

/*--------------------------------------------------------------*/
/** Destroy the semaphore
 * @param[in] sem Semaphore object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_sem_destroy(sem_t *sem);

/*--------------------------------------------------------------*/
/** Convert ms value to the system tick value
 * @param[in] ms Time value in the millisecond
 * @return Sys tick time value
 */
tick_t isix_ms2tick(unsigned long ms);

/*--------------------------------------------------------------*/

/** Wait thread for selected number of ticks
 * @param[in] timeout Wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_wait(tick_t timeout);

/*-----------------------------------------------------------*/
/** Wait thread for selected number of milliseconds
 * @param[in] timeout Wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_wait_ms(unsigned long ms)
{
	return isix_wait(isix_ms2tick(ms));
}

/*-----------------------------------------------------------*/

#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */

/*-----------------------------------------------------------*/
//****************************** C++ API ***********************************
#ifdef __cplusplus
#include <cstddef>
namespace isix {

/*--------------------------------------------------------------*/
//! Semaphore C++ class wrapper
class semaphore
{
public:
	/** Construct semaphore object
	 * @param[in] val Initial value of the semaphore
	 */
	explicit semaphore( int val, int limit_val=ISIX_SEM_ULIMITED )
	{
		sem = isix_sem_create_limited( NULL, val, limit_val );
	}
	//! Destruct semaphore object
	~semaphore()
	{
		isix_sem_destroy(sem);
	}
	/** Check the fifo object is in valid state
	 * @return true if object is in valid state otherwise return false
	 */
	bool is_valid() { return sem!=0; }
	/** Wait for the semaphore for selected time
	 * @param[in] timeout Max waiting time
	 */
	int wait(tick_t timeout)
	{
		return isix_sem_wait( sem, timeout );
	}
	/** Get the semaphore from the ISR context
	 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
	 */
	int get_isr()
	{
		return isix_sem_get_isr(sem);
	}
	/** Signaling the semaphore
	 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
	 */
	int signal()
	{
		return isix_sem_signal(sem);
	}
	/** Signal the semaphore from the ISR context
	 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
	 */
	int signal_isr()
	{
		return isix_sem_signal_isr(sem);
	}
	/** Set value of the semaphore
	 * @param[in] val Value of the semaphore
	 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
	 */
	int setval(int val)
	{
		return isix_sem_setval( sem, val );
	}
	/** Get the semaphore value
	 * @return the semaphore value otherwise an error
	 */
	int getval()
	{
		return isix_sem_getval( sem );
	}
private:
	semaphore(const semaphore&);
	semaphore& operator=(const semaphore&);
private:
	sem_t *sem;
};
/*--------------------------------------------------------------*/
class sem_lock {
public:
	sem_lock( semaphore& _sem )
		: m_sem( _sem ) {
		_sem.wait( ISIX_TIME_INFINITE );
	}
	~sem_lock() {
		m_sem.signal();
	}
private:
	sem_lock(const sem_lock&);
	sem_lock& operator=(const sem_lock&);
	semaphore& m_sem;
};
/*--------------------------------------------------------------*/
}
#endif /* __cplusplus */

/*-----------------------------------------------------------*/
