/*
 * #include <periph/clock/clocks.hpp>
 * =====================================================================================
 *
 *       Filename:  clocks.hpp
 *
 *    Description:  Clock peripherals control
 *
 *        Version:  1.0
 *        Created:  05.03.2018 20:57:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <periph/dt/types.hpp>
#include <periph/clock/clocks_impl.hpp>


namespace periph {
namespace clock {
namespace {

	/** Enable device clock
	 * @param[in] Clock input struct
	 * @return error code
	 */
	inline int device_enable( const dt::clk_periph& clk ) {
		return impl::device_enable( clk );
	}

	/** Enable device clock
	 * @param[in] Clock input struct
	 * @return error code
	 */
	inline int device_disable( const dt::clk_periph& clk ) {
		return impl::device_disable( clk );
	}

	/** Enable device clock
	 * @param[in] Clock input struct
	 * @return error code
	 */
	inline int device_reset( const dt::clk_periph& clk ) {
		return impl::device_reset( clk );
	}

}}}

