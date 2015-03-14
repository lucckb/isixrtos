/*
 * =====================================================================================
 *
 *       Filename:  hw_control.hpp
 *
 *    Description:  Hardware control modem virtual class
 *
 *        Version:  1.0
 *        Created:  28.02.2015 16:34:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

namespace gsm_modem {

	//! Hardware control modem base class like device reset power on and so on
	class hw_control {
	public:
		hw_control( hw_control& ) = delete;
		hw_control& operator=( hw_control& ) = delete;
		hw_control() {
		}
		//! Virtual destructor
		virtual ~hw_control() {
		}
		//! Enable or disable hardware modem device
		virtual void power_control( bool enable ) = 0;
		//! Hardware reset device
		virtual void reset() = 0;
	};
}
