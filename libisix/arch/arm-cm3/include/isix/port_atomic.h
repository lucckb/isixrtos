/*
 * =====================================================================================
 *
 *       Filename:  port_atomic.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/17/2013 10:00:07 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (lb), Lucjan_B1@verifone.com
 *   Organization:  VERIFONE
 *
 * =====================================================================================
 */


#ifndef  _ISIX_PORT_ATOMIC_H
#define  _ISIX_PORT_ATOMIC_H

#include <asm/atomic_ops.h>

/*--------------------------------------------------------------*/

typedef sys_atomic_sem_lock_t _port_atomic_t;

/*--------------------------------------------------------------*/
/** Initialize sem locking primitive 
 * @param [out] lock Lock object
 * @param [in] value Initial semaphore value 
 */
static inline void port_atomic_sem_init( _port_atomic_t* lock, long value ) {
	sys_atomic_sem_init( lock, value );
}

/*--------------------------------------------------------------*/
/** Function try wait on the spinlock semaphore 
 * @param sem[out] Semaphore primitive object
 * @return false if lock failed or positive sem value
 * if semafore is successfuly obtained
 */
static inline int port_atomic_sem_dec( _port_atomic_t* lock ) {
	return sys_atomic_sem_dec( lock );
}
/**
 * Function signal the semaphore
 * @param [out] sem Semaphore primitive object
 */
/*--------------------------------------------------------------*/
/**
 * Function signal the semaphore
 * @param [out] sem Semaphore primitive object
 */
static inline int port_atomic_sem_inc( _port_atomic_t* lock, int maxval ) {
	return sys_atomic_sem_inc( lock , maxval );
}

/*--------------------------------------------------------------*/
/**
 * Clear all exclusive locks
 * @note It should be called during context switching
 */
static inline void port_atomic_clear_locks(void) 
{
	sys_atomic_clear_locks();
}
/*--------------------------------------------------------------*/
/** Sys atomic read value 
 * @param[in] lock Semaphore lock object
 * @return readed value*/
static inline long port_atomic_sem_read_val( sys_atomic_sem_lock_t* lock ) 
{
	return sys_atomic_sem_read_val( lock );
}
/*--------------------------------------------------------------*/
/**
 * Sys atomic write value
 * @param[in] lock Semaphore object input
 * @param[in] val Input value
 * @return Previous value
 */
static inline long port_atomic_sem_write_val( sys_atomic_sem_lock_t* lock, long val )
{
	return sys_atomic_sem_write_val( lock , val );
}
/*--------------------------------------------------------------*/
#endif   /* ----- #ifndef port_atomic_INC  ----- */
