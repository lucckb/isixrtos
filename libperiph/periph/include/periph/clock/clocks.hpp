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

#include <periph/dt/dts.hpp>
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
	/** Check if device has enabled clock signal
	 * @param[in] clk Clock input struct
	 * @return 0 if disabled 1 if enable negative if error
	 */
	inline int device_is_enabled( const dt::clk_periph& clk ) {
		return impl::device_is_enabled( clk );
	}

	/** Enable device clock
	 * @param[in] Clock input struct
	 * @return error code
	 */
	inline int device_reset( const dt::clk_periph& clk ) {
		return impl::device_reset( clk );
	}

	/** Enable or disable device by address and ioctl
	 *  Common hi level API
	 * @param[in] ioaddr Device io address
	 * @param[in] en Enable or disable device
	 * @return error code
	 */
	inline int device_enable(void* const ioaddr, bool en)
	{
		int ret {};
		dt::clk_periph pclk;
		do {
			if((ret=dt::get_periph_clock(ioaddr,pclk))<0) break;
			if((ret=clock::device_is_enabled(pclk))<0) break;
			if(en && ret==0) {
				if((ret=clock::device_enable(pclk))<0) break;
			} else if(!en && ret>0) {
				if((ret=clock::device_disable(pclk))<0) break;
			} else {
				ret = error::success;
			}
		} while(0);
		return ret;
	}


}}}


