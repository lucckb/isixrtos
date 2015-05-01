/*
 * stm32_spi_master.hpp
 * xWildChildx
 *  Created on: 02-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef STM32_SPI_MASTER_HPP_
#define STM32_SPI_MASTER_HPP_

/*----------------------------------------------------------*/
#include "spi_device.hpp"
#include <stm32lib.h>
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif
/*----------------------------------------------------------*/
#define ISIX_DRV_SPI_SPI1_ENABLE (1<<0)
#define ISIX_DRV_SPI_SPI2_ENABLE (1<<1)
#define ISIX_DRV_SPI_SPI3_ENABLE (1<<2)
/*----------------------------------------------------------*/
#ifndef CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS
#define CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS  ISIX_DRV_SPI_SPI1_ENABLE
#endif

/*----------------------------------------------------------*/
namespace stm32 {
namespace drv {

/*----------------------------------------------------------*/
class spi_master : public ::drv::spi_device
{
public:
	/* Constructor */
	explicit spi_master( SPI_TypeDef *spi, unsigned pclk1, unsigned pclk2, bool alternate=false );
	/* Destructor */
	virtual ~spi_master();
	/* Write to the device */
	virtual int write( const void *buf, size_t len);
	virtual int write( const void* buf1, size_t len1,
					   const void* buf2, size_t len2);
	/* Read from the device */
	virtual int read ( void *buf, size_t len);
	/* Transfer (BIDIR) */
	virtual int transfer( const void *inbuf, void *outbuf, size_t len );
	/* Set work mode */
	virtual int set_mode( unsigned mode, unsigned khz );
	/* Setup CRC */
	virtual int crc_setup( unsigned short /*polynominal*/, bool /*enable*/ );
	/* Control CS manually*/
	virtual void CS( bool val, int cs_no );
	/* Transfer data (nodma) */
	virtual uint16_t transfer( uint16_t val );
	/* Disable enable the device */
	virtual void enable( bool en );
protected:
	SPI_TypeDef* const m_spi;
private:
	uint32_t m_pclk;
};

/*----------------------------------------------------------*/
} /* namespace drv */
} /* namespace stm32 */
/*----------------------------------------------------------*/
#endif /* STM32_SPI_MASTER_HPP_ */
/*----------------------------------------------------------*/
