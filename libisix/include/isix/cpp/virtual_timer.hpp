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

#ifdef ISIX_CONFIG_USE_TIMERS
#ifdef __cplusplus


#include <cstddef>
namespace isix {

    //! C++ wrapper for the vtime
    class virtual_timer {
    public:
            //! Create virtual timer object
            virtual_timer() {
                    timer = isix_vtimer_create( vtimer_func, this );
            }
            //! Destroy the virtual timer object
            ~virtual_timer() {
                    isix_vtimer_destroy( timer );
            }
            //! Check that object is valid
            bool is_valid() { return timer!=0; }
            //! Start the timer on selected period
            int start(ostick_t timeout) { return isix_vtimer_start( timer, timeout ); }
            //! Start the timer on selected period
            int start_ms(ostick_t timeout) { return isix_vtimer_start( timer, timeout ); }
            //! Stop the timer
            int stop() { return isix_vtimer_stop( timer ); }
    protected:
            //! Virtual function called on time
            virtual void handle_timer() = 0;
    private:
            static void vtimer_func(void *ptr) {
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
#endif /* ISIX_CONFIG_USE_TIMERS */

