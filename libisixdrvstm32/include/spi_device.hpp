/*
 * spi_device.hpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */

#ifndef ISIXDRV_SPI_DEVICE_HPP_
#define ISIXDRV_SPI_DEVICE_HPP_

#include <cstddef>
#include <stdint.h>
#include <isix.h>
#include <foundation/drv/bus/ibus.hpp>

namespace drv {


/* This is the low level device probably used in the
 * to the other device driver so it need explicit lock unlock to accuire the device
 */
class spi_device : public fnd::drv::bus::ibus
{
public:
	//! Bus address as spi
	enum spi_addr : unsigned {
		CS_ = 0, //! Don't apply chip select
		CS0 = 1, //! Use CS0
	};
	enum data_with
	{
		data_8b  = 0x00,//!< data_8b
		data_16b = 0x01 //!< data_16b
	};
	enum polar
	{
		polar_cpol_low = 0x00,
		polar_cpol_hi  = 0x02
	};
	enum phase
	{
		phase_1edge = 0x00,
		phase_2edge = 0x04
	};
	enum byte_order
	{
		msb_first = 0x00,
		lsb_first = 0x08
	};
	enum cs_conf
	{
		cs_software = 0x00,
		cs_hardware = 0x10
	};
protected:
	//Global timeout for device
	static const unsigned C_spi_timeout = 5000;
public:
	spi_device() : ibus(fnd::drv::bus::ibus::type::spi)
	{}
	virtual ~spi_device() {}
	/* Flush bytes */
	void flush(size_t elems) {
		for(size_t e = 0; e<elems; ++e )
			transfer(0xFFFF);
	}
	/* Disable enable the device */
	virtual void enable( bool en ) = 0;
	/** Select mode for the chip select
	 * @param[in] mode Bus mode
	 * @param[in] khz Speed in khz
	 * @return error code
	 */
	virtual int set_mode( unsigned mode, unsigned khz, int cs=CS0 ) = 0;
	/* Setup CRC */
	virtual int crc_setup( unsigned short /*polynominal*/, bool /*enable*/ )
	{
		return err_not_supported;
	}
	virtual void mdelay( unsigned ms ) noexcept override {
		isix::wait_ms(ms);
	}
	/* Control CS manually*/
	virtual void CS( bool val, int cs_no ) = 0;
	/* Transfer data (nodma) */
	virtual uint16_t transfer( uint16_t val ) = 0;
};


}

#endif /* SPI_DEVICE_HPP_ */
