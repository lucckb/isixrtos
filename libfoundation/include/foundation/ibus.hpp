/*
 * =====================================================================================
 *
 *       Filename:  i2c_bus.hpp
 *
 *    Description:  I2C bus implementation
 *
 *        Version:  1.0
 *        Created:  06.03.2014 18:54:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include "noncopyable.hpp"
#include <cstddef>

namespace fnd {
namespace bus {

//I2C virtual bus
class ibus : private noncopyable {
public:
	//! Error code
	enum err {
		err_ok = 0,						//! all is ok
		err_bus = -1024,				//! bus error
		err_arbitration_lost = -1025,
		err_ack_failure = -1026,
		err_overrun = -1027,
		err_pec = -1028,				//! parity check error
		err_bus_timeout = -1029,		//! bus timeout
		err_timeout = -1030,			//! timeout error
		err_invstate = -1031,			//! Invalid machine state
		err_invaddr = -1032,			//! Invalid address
		err_unknown = -1033				//! Unknown error
	};
	//! Destructor
	virtual ~ibus() {

	}
	/** Transfer data over the bus
	 * @param[in] addr I2C address
	 * @param[in] wbuffer Memory pointer for write
	 * @param[in] wsize  Size of write buffer
	 * @param[out] rbuffer Read data buffer pointer
	 * @param[in] rsize Read buffer sizes
	 * @return Error code or success */
	virtual int
		transfer( unsigned  address, const void* wbuffer, size_t wsize, void* rbuffer, size_t rsize ) = 0;
	/** Double non continous transaction write 
	 * @param[in] addr I2C address
	 * @param[in] wbuf1 Write buffer first transaction
	 * @param[in] wsize1 Transaction size 1
	 * @param[in] wbuf2 Write buffer first transaction
	 * @param[in] wsize2 Transaction size 1
	 * @return error code or success */
	virtual int
		write( unsigned addr, const void* wbuf1, size_t wsize1, const void* wbuf2, size_t wsize2 ) = 0;

	//! Delay amount of msec
	virtual void mdelay( unsigned ms ) = 0;
};


} }

