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
	int write( const void *buf, size_t len) override;
	int write( const void* buf1, size_t len1,
					   const void* buf2, size_t len2) override;
	/* Read from the device */
	int read ( void *buf, size_t len) override;
	/* Transfer (BIDIR) */
	int transfer( const void *inbuf, void *outbuf, size_t len ) override;
	/* Set work mode */
	int set_mode( unsigned mode, unsigned khz ) override;
	/* Setup CRC */
	int crc_setup( unsigned short /*polynominal*/, bool /*enable*/ ) override;
	/* Control CS manually*/
	void CS( bool val, int cs_no ) override;
	/* Transfer data (nodma) */
	uint16_t transfer( uint16_t val ) override {
		if( m_8bit ) return transfer8( val );
		else return transfer16( val );
	}
	/* Disable enable the device */
	void enable( bool en ) override;
private:
	uint16_t transfer16( uint16_t val );
	uint8_t transfer8( uint8_t val );
protected:
	SPI_TypeDef* const m_spi;
private:
	uint32_t m_pclk;
	const bool m_alt;
	bool m_8bit { true };
};

/*----------------------------------------------------------*/
} /* namespace drv */
} /* namespace stm32 */
/*----------------------------------------------------------*/
#endif /* STM32_SPI_MASTER_HPP_ */
/*----------------------------------------------------------*/
