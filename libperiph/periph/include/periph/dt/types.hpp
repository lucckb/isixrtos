/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  Device tree types
 *
 *        Version:  1.0
 *        Created:  05.03.2018 21:03:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

namespace periph::dt {

	//! Available buses
	enum class bus : short {
		_empty,
		axi,
		ahb1,
		apb1,
		apb2,
		cpu
	};

	//! Pin function type
	enum class pinfunc : short {
		_empty,
		sck, miso, mosi,
		cs0, cs1, cs2, cs3,
		rxd,
		txd,
	};

	inline pinfunc operator++(pinfunc pf) {
		return static_cast<pinfunc>(static_cast<int>(pf)+1);
	}

	//! Clk periph information structure
	struct clk_periph {
		bus xbus;				//!	Assigned bus
		unsigned bit;			//! Peripheral bit number for enable device
	};

	//! Device configuration
	struct device_conf {
		int irqnum;				//! IRQ number
		unsigned irqconf;		//! IRQ priotity
		unsigned flags;			//! Device dma flags
	};

}



