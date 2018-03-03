/*
 * error.hpp
 *
 *  Created on: 21 sty 2018
 *      Author: lucck
 */

#pragma once

namespace periph {
namespace error {


enum basic_errors {
	success	= 0,
	nodev  = -128,
	nopin  = -129,
	noclk  = -130
};

//! Bus errors
enum bus_errors {
	arbitration_lost = -512,	//! bus arbitration lost
	ack_failure = -513,			//! acknowledge failure
	overrun = -514,				//! Buss overrun
	pec = -515,					//! parity check error
	bus_timeout = -516,			//! bus timeout
	timeout = -517,				//! timeout error
	invstate = -518,			//! Invalid machine state
	invaddr = -519,				//! Invalid address
	unknown = -520,				//! Unknown error
	not_supported = -521,		//! Not supported
	hw = -522,					//! Internal hardware error
	noinit = -523,				//! Driver not initialized
	dma = -524,					//! DMA error
	inval = -525				//! Invalid argument
};


}}
