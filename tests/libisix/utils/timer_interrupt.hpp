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


#pragma once

#include <cstdint>
#include <functional>


namespace tests {
namespace detail {

	//Default handler function
	using timer_handler_t = std::function<void()>;
	//Periodic timer setup
	void periodic_timer_setup( timer_handler_t normal, uint16_t timeval );
	//Stop the periodic timer
	void periodic_timer_stop() noexcept;
}}

