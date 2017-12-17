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
#include <cstddef>

namespace fnd {
namespace drv {
namespace bus {

//! Hardware bus interface for all devices
class ibus
{
public:
	ibus(ibus&) = delete;
	ibus& operator=(ibus&) = delete;
public:
	//! Error code
	enum err {
		err_ok = 0,						//! all is ok
		err_bus = -1024,				//! bus error
		err_arbitration_lost = -1025,	//! bus arbitration lost
		err_ack_failure = -1026,		//! acknowledge failure
		err_overrun = -1027,			//! Buss overrun
		err_pec = -1028,				//! parity check error
		err_bus_timeout = -1029,		//! bus timeout
		err_timeout = -1030,			//! timeout error
		err_invstate = -1031,			//! Invalid machine state
		err_invaddr = -1032,			//! Invalid address
		err_unknown = -1033,			//! Unknown error
		err_not_supported = -1034,		//! Not supported
		err_hw = -1035,					//! Internal hardware error
		err_noinit = -1036,				//! Driver mot initialized
		err_dma = -1037,				//! DMA error
		err_inval = -1038				//! Invalid argument
	};
	//! Get bus type
	enum class type : unsigned char {
		i2c,
		spi,
	};
	//! Constructor
	explicit ibus(type bus_type)
		: m_bus_type(bus_type)
	{
	}
	//! Destructor
	virtual ~ibus() {
	}
	/** Get bus type without rtti
	 */
	type bus_type() const noexcept {
		return m_bus_type;
	}
	/** Transfer data write and read next
	 * @param[in] addr Hardware address
	 * @param[in] wbuffer Memory pointer for write
	 * @param[in] wsize  Size of write buffer
	 * @param[out] rbuffer Read data buffer pointer
	 * @param[in] rsize Read buffer sizes
	 * @return Error code or success */
	virtual int
		transfer(unsigned /*address*/, const void* /*wbuffer*/, size_t /*wsize*/, void* /*rbuffer*/, size_t /*rsize*/)
	{
		return err_not_supported;
	}
	/**
	 * Transfer full duplex data
	 * @param addr Hardware address
	 * @param inbuf  Buffer for data in
	 * @param outbuf Buffer for data out
	 * @param len Transfer size
	 * @return
	 */
	virtual int
		transfer( unsigned /*addr*/, const void* /*inbuf*/, void* /*outbuf*/, size_t /*len*/ )
	{
		return err_not_supported;
	}
	/** Write data to the memory buffer
	 * @param addr Hardware address
	 * @param wrbuf Write buffer
	 * @param size Write size
	 * @return Error code
	 */
	virtual int write( unsigned addr, const void* wrbuf, size_t size ) = 0;
	/** Double non continous transaction write
	 * @param[in] addr I2C address
	 * @param[in] wbuf1 Write buffer first transaction
	 * @param[in] wsize1 First transaction buffer size
	 * @param[in] wbuf2 Write buffer second transaction
	 * @param[in] wsize2 Second transaction buffer
	 * @return error code or success */
	virtual int
		write(unsigned addr, const void* wbuf1, size_t wsize1, const void* wbuf2, size_t wsize2) = 0;
	/**
	 * Read data from the memory
	 * @param[in] addr Hardware address
	 * @param[out] rdbuf Read buffer pointer
	 * @param size Transfer length
	 * @return Error code
	 */
	virtual int read( unsigned addr, void* rdbuf, size_t size ) = 0;
	/**
	 * Delay milisec
	 * @param delay selected ms
	 */
	virtual void mdelay( unsigned ms ) noexcept = 0;

private:
	type m_bus_type;

};

} } }

