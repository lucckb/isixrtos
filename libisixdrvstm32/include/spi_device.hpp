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
	static constexpr auto max_cs = 4;
	//! SPI configuration device
	union config_t {
		struct {
			unsigned flags: 8;
			unsigned  speed: 24;
		};
		unsigned packed;
	};
	//! Bus address as spi
	enum spi_addr : int {
		CS0, CS1, CS2, CS3,
		CS_, //! Don't apply chip select
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
private:
	//Global timeout for device
	static const unsigned C_spi_timeout = 5000;
	//Reconfiguration needed
	bool mode_reconf_needed(int cs) const noexcept
	{
		return cs>=CS0 && cs<=CS3 &&
			m_cs_modes[cs-CS0].packed!=m_current_mode.packed;
	}
public:
	/** Create SPI device with number of cs
	 * lines
	 */
	spi_device()
		: ibus(fnd::drv::bus::ibus::type::spi)
	{
	}
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
	 */
	int set_mode( unsigned mode, unsigned khz, int cs=CS0 ) noexcept
	{
		if( cs < CS0 || cs > CS3 ) return err_invaddr;
		config_t cnf { uint8_t(mode), khz };
		m_cs_modes[cs] = cnf;
		return err_ok;
	}
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
protected:
	//! Hardware reconfigure mode
	 virtual void hw_set_mode( unsigned mode, unsigned khz ) noexcept = 0;
private:
	/* Current config mode */
	config_t m_current_mode {};
	/* Vector for chip select config */
	std::array<config_t,max_cs> m_cs_modes {{}};
};

//! Hardware control chip select
inline void spi_device::CS( bool val, int cs_no )
{
	if( val ) return;
	//Switch only when 0
	if( mode_reconf_needed(cs_no) ) {
		hw_set_mode(m_cs_modes[cs_no].flags,m_cs_modes[cs_no].speed);
		m_current_mode = m_cs_modes[cs_no];
	}
}

}

#endif /* SPI_DEVICE_HPP_ */
