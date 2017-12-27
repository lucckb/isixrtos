/*
 * stm32_spi_master.hpp
 * xWildChildx
 *  Created on: 02-12-2012
 *      Author: lucck
 */

#ifndef STM32_SPI_MASTER_HPP_
#define STM32_SPI_MASTER_HPP_


#include "spi_device.hpp"
#include <stm32lib.h>
#include <vector>
#include <config/conf.h>

#define ISIX_DRV_SPI_SPI1_ENABLE (1<<0)
#define ISIX_DRV_SPI_SPI2_ENABLE (1<<1)
#define ISIX_DRV_SPI_SPI3_ENABLE (1<<2)

#ifndef CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS
#define CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS  ISIX_DRV_SPI_SPI1_ENABLE
#endif


namespace stm32 {
namespace drv {


using spi_cs_conf = std::array<gpio::pin_desc,4>;

//! Gpio port config
struct spi_gpio_config {
	unsigned pclk1;
	unsigned pclk2;
	unsigned alt;
	gpio::pin_desc miso;
	gpio::pin_desc mosi;
	gpio::pin_desc sck;
	spi_cs_conf cs;			//null end off
};


class spi_master : public ::drv::spi_device
{
public:
	/* Constructor */
	spi_master( SPI_TypeDef *spi, unsigned pclk1, unsigned pclk2, bool alternate=false );
	spi_master( SPI_TypeDef *spi, const spi_gpio_config& iocnf );
	/* Destructor */
	virtual ~spi_master();
	/* Write to the device */
	int write( unsigned addr, const void *buf, size_t len) override;
	int write( unsigned addr, const void* buf1, size_t len1,
					   const void* buf2, size_t len2) override;
	/* Read from the device */
	int read ( unsigned addr, void *buf, size_t len) override;
	/* Transfer (BIDIR) */
	int transfer( unsigned addr, const void *inbuf, void *outbuf, size_t len ) override;

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
protected:
	/** SPI internal set mode */
	 void hw_set_mode( unsigned mode, unsigned khz ) noexcept override;
private:
	uint16_t transfer16( uint16_t val );
	uint8_t transfer8( uint8_t val );
protected:
	SPI_TypeDef* const m_spi;
private:
	uint32_t m_pclk;
	const bool m_alt;
	const bool m_newapi;
	bool m_8bit { true };
	spi_cs_conf m_cs;
};


} /* namespace drv */
} /* namespace stm32 */

#endif /* STM32_SPI_MASTER_HPP_ */

