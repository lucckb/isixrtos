/*
 * =====================================================================================
 *
 *       Filename:  port_atomic.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/17/2013 10:00:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (lb)
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef  _ISIX_PORT_ATOMIC_H
#define  _ISIX_PORT_ATOMIC_H

/*--------------------------------------------------------------*/
#include <asm/atomic_sem.h>
#include <asm/atomic.h>

/*--------------------------------------------------------------*/
//! Atomic semaphore type definition
typedef sys_atomic_sem_lock_t _port_atomic_t;

//! Atomic counter type
typedef sys_atomic_t _port_atomic_int_t;

/*--------------------------------------------------------------*/
/** Initialize sem locking primitive 
 * @param [out] lock Lock object
 * @param [in] value Initial semaphore value 
 */
static inline void port_atomic_sem_init( _port_atomic_t* lock, int value , int limit ) {
	sys_atomic_sem_init( lock, value, limit );
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
static inline int port_atomic_sem_inc( _port_atomic_t* lock ) {
	return sys_atomic_sem_inc( lock );
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
static inline long port_atomic_sem_read_val( _port_atomic_t* lock ) 
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
static inline int port_atomic_sem_write_val( _port_atomic_t* lock, int val )
{
	return sys_atomic_sem_write_val( lock , val );
}
/*--------------------------------------------------------------*/
/** 
 *	Port atomic increment
 *	@param[in] val Atomic type
 *	@return returned value
 */
static inline int port_atomic_inc( _port_atomic_int_t* val ) 
{
	return sys_atomic_add_return( 1, val );
}
/*--------------------------------------------------------------*/
/** 
 *	Port atomic decrement
 *	@param[in] val Atomic type
 *	@return returned value
 */
static inline int port_atomic_dec( _port_atomic_int_t* val ) 
{
	return sys_atomic_sub_return( 1, val );
}
/*--------------------------------------------------------------*/
/**
 * Sys atomic read value
 * @param[in] val Atomic type
 * @return counter value
 */
static inline int port_atomic_read( _port_atomic_int_t* val )
{
	return sys_atomic_read( val );
}
/*--------------------------------------------------------------*/
/** 
 *	Port atomic increment
 *	@param[in] val Atomic type
 *	@return returned value
 */
static inline unsigned port_unsigned_atomic_inc( _port_atomic_int_t* val ) 
{
	return (unsigned)sys_atomic_add_return( 1, val );
}
/*--------------------------------------------------------------*/
/** 
 *	Port atomic decrement
 *	@param[in] val Atomic type
 *	@return returned value
 */
static inline unsigned port_unsigned_atomic_dec( _port_atomic_int_t* val ) 
{
	return (unsigned)sys_atomic_sub_return( 1, val );
}
/*--------------------------------------------------------------*/
/**
 * Sys atomic read value
 * @param[in] val Atomic type
 * @return counter value
 */
static inline unsigned port_unsigned_atomic_read( _port_atomic_int_t* val )
{
	return (unsigned)sys_atomic_read( val );
}
/*--------------------------------------------------------------*/
static inline void port_atomic_wait_for_interrupt()
{
	sys_atomic_wait_for_interrupt();
}
/*--------------------------------------------------------------*/
#endif   /* ----- #ifndef port_atomic_INC  ----- */
