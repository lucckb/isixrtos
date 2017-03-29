/*
 * =====================================================================================
 *
 *       Filename:  virtual_timer.hpp
 *
 *    Description:  Virtual timer reimpl
 *
 *        Version:  1.0
 *        Created:  02.04.2015 22:58:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#if CONFIG_ISIX_USE_TIMERS
#ifdef __cplusplus


#include <cstddef>
namespace isix {

    //! C++ wrapper for the vtime
    class virtual_timer {
    public:
            //! Create virtual timer object
            virtual_timer() {
				timer = vtimer_create( );
            }
            //! Destroy the virtual timer object
            ~virtual_timer() {
				vtimer_destroy( timer );
            }
            //! Check that object is valid
            bool is_valid() { 
				return timer!=0; 
			}
            //! Start the timer on selected period
            int start( ostick_t timeout, bool cyclic=true ) { 
				return vtimer_start( timer, callback, this, timeout, cyclic ); 
			}
            //! Start the timer on selected period
            int start_ms( ostick_t timeout, bool cyclic=true ) { 
				return vtimer_start( timer, callback, this, ms2tick(timeout), cyclic ); 
			}
            //! Stop the timer
            int stop() { 
				return isix_vtimer_cancel( timer ); 
			} 
			//! Start the timer on selected period
            int start_isr( ostick_t timeout, bool cyclic=true ) { 
				return vtimer_start_isr( timer, callback, this, timeout, cyclic ); 
			}
            //! Start the timer on selected period
            int start_ms_isr( ostick_t timeout, bool cyclic=true ) { 
				return vtimer_start_isr( timer, callback, this, ms2tick(timeout), cyclic ); 
			}
            //! Stop the timer
            int stop_isr() { 
				return isix_vtimer_cancel_isr( timer ); 
			}

    protected:
            //! Virtual function called on time
            virtual void handle_timer() = 0;
    private:
            static void callback(void *ptr) {
				static_cast<virtual_timer*>(ptr)->handle_timer();
            }
    private:
            //Noncopyable
            virtual_timer(const virtual_timer&);
            virtual_timer& operator=(const virtual_timer&);
    private:
            osvtimer_t timer;
    };

}

#endif /*  __cplusplus */
#endif /* CONFIG_ISIX_USE_TIMERS */

