/*
 * =====================================================================================
 *
 *       Filename:  spinlock.h
 *
 *    Description: 	Spinlock inplementation for isix RTOS
 *    				Especially when waiting on spinlock
 *    				reschedule task shuld be disabled for prevent priotity
 *    				inversion on the OS
 *
 *        Version:  1.0
 *        Created:  21.12.2013 16:16:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

/*-----------------------------------------------------------------------*/
#ifndef  ISIX_SPINLOCK_H
#define  ISIX_SPINLOCK_H
/*------------------------------------------------------*/
#include <isix/types.h>
#include <isix/port_atomic.h>
/*------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/
/*-----------------------------------------------------------------------*/
/** Temporary lock task reschedule */
void _isixp_lock_scheduler();

/*-----------------------------------------------------------------------*/
/** Temporary unlock task reschedule */
void _isixp_unlock_scheduler();

/*-----------------------------------------------------------------------*/
//! Isix spinlock type 
typedef struct 
{
	_port_atomic_t sem;
} isix_spinlock_t;
/*-----------------------------------------------------------------------*/
/** Spinlock initialize 
 * @param[in] lock Spinlock object
 */
static inline void isix_spinlock_init( isix_spinlock_t* lock ) 
{
	lock->sem.limit = 1;
	lock->sem.value = 1;
}
/*-----------------------------------------------------------------------*/
/**
 *	Spinlock lock in busywait modea
 *	It lock the current task and waits for free system resource
 *	@param[in] lock Spinlock object
 */
static inline void isix_spinlock_lock( isix_spinlock_t* lock )
{
	if( port_atomic_sem_dec( &lock->sem ) == 0 ) {
		_isixp_lock_scheduler();
		while( port_atomic_sem_read_val( &lock->sem ) == 0 ) {
			port_atomic_wait_for_interrupt();
		}
		_isixp_unlock_scheduler();
	}
}
/*-----------------------------------------------------------------------*/
/** Its unlock the locked resource  */
static inline void isix_spinlock_unlock( isix_spinlock_t* lock )
{
	port_atomic_sem_inc( &lock->sem );
}
/*-----------------------------------------------------------------------*/
static inline int isix_spinlock_try_lock( isix_spinlock_t* lock )
{
	return port_atomic_sem_dec( &lock->sem );
}
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
#endif   /* ----- #ifndef spinlock_INC  ----- */
