/*
 * =====================================================================================
 *
 *       Filename:  utility.h
 *
 *    Description:  Utility API
 *
 *        Version:  1.0
 *        Created:  04.04.2015 23:25:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/arch/ostimer.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/** Get current sytem ticks
 * @return Number of system tick from system startup in usec resolution
 */
static inline osutick_t isix_get_ujiffies(void)
{
	osutick_t t = (osutick_t)isix_get_jiffies() * ((osutick_t)1000000/((osutick_t)ISIX_HZ));
    t += (((osutick_t)_isix_port_get_hres_jiffies_timer_value()) * ((osutick_t)1000000/((osutick_t)ISIX_HZ)))
    	/  (osutick_t)_isix_port_get_hres_jiffies_timer_max_value();
    return t;
}

/** Busy waiting for selecred amount of time
 * @param[in] timeout Number of microseconds for busy wait
 * @return None
 */
void isix_wait_us( unsigned timeout );

/** Check if isix timer elapsed or not
 *   @param[in] t1 Variable with the initial timer value
     @param[in] timeout Excepted timeout
	 @return true if timer elapsed otherwise false
*/
static inline bool isix_timer_elapsed( ostick_t t1, ostick_t timeout )
{
	ostick_t t2 = isix_get_jiffies();
	if( t2 >= t1 )	//Not overflow
		return t2 - t1 > timeout;
	else       //Overflow
		return t1 - t2 > timeout;
}

/** Convert ms value to the system tick value
 * @param[in] ms Time value in the millisecond
 * @return Sys tick time value
 */
ostick_t isix_ms2tick( unsigned long ms );


/** Wait thread for selected number of ticks
 * @param[in] timeout Wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_wait( ostick_t timeout );

/** Wait thread for selected number of milliseconds
 * @param[in] timeout Wait time
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_wait_ms( unsigned long ms )
{
	return isix_wait(isix_ms2tick(ms));
}


#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */



#ifdef __cplusplus
namespace isix {
namespace {
	inline void wait_us( unsigned timeout ) {
		return ::isix_wait_us( timeout );
	}
	inline osutick_t get_ujiffies() {
		return ::isix_get_ujiffies();
	}
	inline bool timer_elapsed( ostick_t t1, ostick_t timeout ) {
		return ::isix_timer_elapsed( t1, timeout );
	}
	inline ostick_t ms2tick( unsigned long ms ) {
		return ::isix_ms2tick( ms );
	}
	inline int wait( ostick_t timeout ) {
		return ::isix_wait( timeout );
	}
	inline int wait_ms( unsigned long ms ) {
		return ::isix_wait_ms( ms );
	}
}}
#endif /* __cplusplus */
