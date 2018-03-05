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

namespace periph {
namespace dt {

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
		sck,
		miso,
		mosi,
		rxd,
		txd,
	};

	//! Clk periph information structure
	struct clk_periph {
		bus xbus;				//!	Assigned bus
		unsigned bit;			//! Peripheral bit number for enable device
	};



}}



