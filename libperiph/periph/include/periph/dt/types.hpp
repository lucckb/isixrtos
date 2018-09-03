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
        unspec,
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
		rxd, txd,
        rst, rw
	};

	//++ operator
	inline pinfunc operator++(pinfunc& pf) {
		pf = static_cast<pinfunc>(static_cast<int>(pf)+1);
		return pf;
	}

	//! Clk periph information structure
	struct clk_periph {
		bus xbus;				//!	Assigned bus
		unsigned bit;			//! Peripheral bit number for enable device
	};

    struct device_conf_base {
    };

	//! Device configuration internal hardware
	struct device_conf : public device_conf_base {
		int irqnum;				//! Irq number
		unsigned short irqfl;	//! Irq low flags
		unsigned short irqfh;	//! Irq hi con
        unsigned long flags;		//! Base flags
	};
    
}
