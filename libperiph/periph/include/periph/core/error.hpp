/*
 * error.hpp
 *
 *  Created on: 21 sty 2018
 *      Author: lucck
 */

#pragma once
#include "exceptions.hpp"

namespace periph::error {


enum generic_errors {
	success	= 0,
	nodev  = -128,
	nopin  = -129,
	noclk  = -130,
	nobus  = -131,
	init   = -132,
	again  = -133,
	exists = -134,
	nosys  = -135,
	noent = -136,
	unimplemented = -137,
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
	inval = -525,				//! Invalid argument
	busy  = -526,				//! Controler is busy
	bus  = -527					//! Generic bus error
};


}
