/*
 * =====================================================================================
 *
 *       Filename:  timer_interrupt.hpp
 *
 *    Description:  Timer interrupt API handling for testing IRQ primitives
 *
 *        Version:  1.0
 *        Created:  12.12.2013 22:44:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef  timer_interrupt_INC
#define  timer_interrupt_INC

#include <cstdint>
#include <functional>


namespace tests {
namespace detail {

	//Default handler function 
	using timer_handler_t = std::function<void()>;
	//Periodic timer setup
	void periodic_timer_setup( timer_handler_t normal, uint16_t timeval = 65535, 
			timer_handler_t nested = nullptr 
	) noexcept ;

	//Stop the periodic timer 
	void periodic_timer_stop() noexcept;

}}

#endif   /* ----- #ifndef timer_interrupt_INC  ----- */
