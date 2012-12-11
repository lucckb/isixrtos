/*
 * spi_device.hpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef ISIXDRV_SPI_DEVICE_HPP_
#define ISIXDRV_SPI_DEVICE_HPP_
/*----------------------------------------------------------*/
#include <cstddef>
#include <stdint.h>
#include <isix.h>
#include <noncopyable.hpp>
/*----------------------------------------------------------*/
namespace drv {

/*----------------------------------------------------------*/
/* This is the low level device probably used in the
 * to the other device driver so it need explicit lock unlock to accuire the device
 */
class spi_device  :  private fnd::noncopyable
{
public:
	enum err {
		err_ok,
		err_not_supported,
		err_hw,
		err_noinit
	};
	enum data_with
	{
		data_8b  = 0x00,
		data_16b = 0x01
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
	spi_device()
		: m_lksem( 1, 1 )
	{}
	/* Flush bytes */
	void flush(size_t elems)
	{
		for(size_t e = 0; e<elems; ++e )
			transfer(0xFFFF);
	}
	/* Lock SPI bus */
	int lock( unsigned timeout )
	{
		return m_lksem.wait( timeout );
	}
	/* Unlock SPI BUS */
	int unlock()
	{
		return m_lksem.signal();
	}
	/* Write to the device */
	virtual int write( const void *buf, size_t len ) = 0;
	/* Read from the device */
	virtual int read ( void *buf, size_t len ) = 0;
	/* Transfer (BIDIR) */
	virtual int transfer( const void *inbuf, void *outbuf, size_t len ) = 0;
	/* Disable enable the device */
	virtual void enable( bool en ) = 0;
	/* Set work mode */
	virtual int set_mode( unsigned mode, unsigned khz ) = 0;
	/* Setup CRC */
	virtual int crc_setup( unsigned short /*polynominal*/, bool /*enable*/ )
	{
		return err_not_supported;
	}
	/* Control CS manually*/
	virtual void CS( bool val, int cs_no ) = 0;
	/* Transfer data (nodma) */
	virtual uint16_t transfer( uint16_t val ) = 0;
private:
	isix::semaphore m_lksem;	//Lock semaphore
};

/*----------------------------------------------------------*/
//SPI device locker
class spi_device_autolock
{
public:
	spi_device_autolock( spi_device &dev )
		: m_dev(dev)
	{
		m_dev.lock(isix::ISIX_TIME_INFINITE);
	}
	~spi_device_autolock()
	{
		m_dev.unlock();
	}
private:
	spi_device& m_dev;
};
/*----------------------------------------------------------*/
}
/*----------------------------------------------------------*/
#endif /* SPI_DEVICE_HPP_ */
