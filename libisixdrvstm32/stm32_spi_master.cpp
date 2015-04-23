/*
 * stm32_spi_master.cpp
 *
 *  Created on: 02-12-2012
 *      Author: lucck
 */

#include "stm32_spi_master.hpp"
#include <stm32rcc.h>
#include <stm32spi.h>
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif


namespace stm32 {
namespace drv {

namespace {
namespace spi1 {
	const uint16_t SD_SPI_MISO_PIN	= 6;
	const uint16_t SD_SPI_MOSI_PIN	= 7;
	const uint16_t SD_SPI_SCK_PIN   = 5;
	const uint16_t SD_SPI_CS_PIN 	= 4;
	GPIO_TypeDef* const SPI_PORT = GPIOA;
}
}

/* Constructor */
spi_master::spi_master( SPI_TypeDef *spi, unsigned pclk1, unsigned pclk2, bool alternate )
	: m_spi( spi ), m_pclk( spi==SPI1?pclk2:pclk1)
{
	using namespace stm32;
	if( m_spi == SPI1 )
	{
		using namespace spi1;
		rcc_apb2_periph_clock_cmd( RCC_APB2Periph_SPI1, true );
		/* Configure SPI1 to use it as SPI dev */
		if( !alternate ) {
			gpio_clock_enable( SPI_PORT, true );
			gpio_abstract_config( SPI_PORT, SD_SPI_SCK_PIN,  
					AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			gpio_abstract_config( SPI_PORT, SD_SPI_MOSI_PIN, 
					AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
			gpio_abstract_config( SPI_PORT, SD_SPI_MISO_PIN, 
					AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
			gpio_abstract_config( SPI_PORT, SD_SPI_CS_PIN,   
					AGPIO_MODE_OUTPUT_PP, AGPIO_SPEED_HALF );
		} else {
			//TODO: Add alternate support 
		}
	}
#ifdef SPI2
	else if( m_spi == SPI2 )
	{
		stm32::rcc_apb1_periph_clock_cmd( RCC_APB1Periph_SPI2, true );
	}
#endif
#ifdef SPI3
	else if( m_spi == SPI3 )
	{
		stm32::rcc_apb1_periph_clock_cmd( RCC_APB1Periph_SPI3, true );
	}
#endif
}

/* Destructor */
spi_master::~spi_master()
{
	using namespace stm32;
	spi_cmd( m_spi, false );
	if( m_spi == SPI1 )
	{
		using namespace spi1;
		gpio_abstract_config( SPI_PORT, SD_SPI_SCK_PIN,  AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
		gpio_abstract_config( SPI_PORT, SD_SPI_MOSI_PIN, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
		gpio_abstract_config( SPI_PORT, SD_SPI_MISO_PIN, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
		gpio_abstract_config( SPI_PORT, SD_SPI_CS_PIN,   AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
		rcc_apb2_periph_clock_cmd( RCC_APB2Periph_SPI1, false );
	}
#ifdef SPI2
	else if( m_spi == SPI2 )
	{
		stm32::rcc_apb1_periph_clock_cmd( RCC_APB1Periph_SPI2, false );
	}
#endif
#ifdef SPI3
	else if( m_spi == SPI3 )
	{
		stm32::rcc_apb1_periph_clock_cmd( RCC_APB1Periph_SPI3, false );
	}
#endif
}

/* Write to the device */
int spi_master::write( const void *buf, size_t len )
{
	if( !stm32::spi_is_enabled(m_spi) )
		return spi_device::err_noinit;
	if ( m_spi->CR1 & SPI_DataSize_16b )
	{
		const uint16_t *b = static_cast<const uint16_t*>(buf);
		len /= 2;
		for(size_t p=0; p<len; p++ )
		{
			transfer( b[p]);
		}
	}
	else
	{
		//dbprintf("Begin WRITE %i", len);
		const uint8_t *b = static_cast<const uint8_t*>(buf);
		for(size_t p=0; p<len; p++ )
		{
			transfer( b[p]);
			//dbprintf("%02x", b[p]);
		}
	}
	return 0;
}

int spi_master::write( const void* buf1, size_t len1,
					const void* buf2, size_t len2 ) 
{
	int ret;
	do {
		ret = write( buf1, len1 );
		if( ret ) break;
		ret = write( buf2, len2 );
		if( ret ) break;
	} while(0);
	return ret;
}

/* Read from the device */
int spi_master::read ( void *buf, size_t len)
{
	if( !stm32::spi_is_enabled(m_spi) )
		return spi_device::err_noinit;
	if ( m_spi->CR1 & SPI_DataSize_16b )
	{
		uint16_t *b = static_cast<uint16_t*>(buf);
		len /= 2;
		for(size_t p=0; p<len; p++ )
		{
			b[p] = transfer( 0xff );
		}
	}
	else
	{
		uint8_t *b = static_cast<uint8_t*>(buf);
		for(size_t p=0; p<len; p++ )
		{
			b[p] = transfer( 0xff );
		}
	}
	return 0;
}

/* Transfer (BIDIR) */
int spi_master::transfer( const void *inbuf, void *outbuf, size_t len )
{
	if( !stm32::spi_is_enabled(m_spi) )
		return spi_device::err_noinit;
	if ( m_spi->CR1 & SPI_DataSize_16b )
	{
		const uint16_t *ib = static_cast<const uint16_t*>(inbuf);
		uint16_t *ob = static_cast<uint16_t*>(outbuf);
		len /= 2;
		for(size_t p=0; p<len; p++ )
		{
			ob[p] = transfer( ib[p] );
		}
	}
	else
	{
		const uint8_t *ib = static_cast<const uint8_t*>(inbuf);
		uint8_t *ob = static_cast<uint8_t*>(outbuf);
		for(size_t p=0; p<len; p++ )
		{
			ob[p] = transfer( ib[p] );
		}
	}
	return 0;
}

/* Set work mode */
int spi_master::set_mode( unsigned mode, unsigned khz )
{
	using namespace stm32;
	if( !khz )
		return spi_device::err_inval;
	int divide = (m_pclk/1000) / khz;
	if( divide <= 2 )
	{
		divide = SPI_BaudRatePrescaler_2;
	}
	else if( divide > 2 && divide <= 4 )
	{
		divide = SPI_BaudRatePrescaler_4;
	}
	else if( divide > 4 && divide <= 8 )
	{
		divide = SPI_BaudRatePrescaler_8;
	}
	else if( divide > 8 && divide <= 16 )
	{
		divide = SPI_BaudRatePrescaler_16;
	}
	else if( divide > 16 && divide <= 32 )
	{
		divide = SPI_BaudRatePrescaler_32;
	}
	else if( divide > 32 && divide <= 64 )
	{
		divide = SPI_BaudRatePrescaler_64;
	}
	else if( divide > 64 && divide <= 128 )
	{
		divide = SPI_BaudRatePrescaler_128;
	}
	else if( divide > 128 )
	{
		divide = SPI_BaudRatePrescaler_256;
	}
	spi_cmd( m_spi, false );
	spi_init( m_spi,  SPI_Direction_2Lines_FullDuplex, SPI_Mode_Master,
		(mode&spi_device::data_16b)?(SPI_DataSize_16b):(SPI_DataSize_8b),
		(mode&spi_device::polar_cpol_hi)?(SPI_CPOL_High):(SPI_CPOL_Low),
		(mode&spi_device::phase_2edge)?(SPI_CPHA_2Edge):(SPI_CPHA_1Edge),
		(mode&spi_device::cs_hardware)?(SPI_NSS_Hard):(SPI_NSS_Soft),
			divide,
		(mode&spi_device::lsb_first)?(SPI_FirstBit_LSB):(SPI_FirstBit_MSB), -1
	);
	spi_cmd( m_spi, true );
	return 0;
}

/* Setup CRC */
int spi_master::crc_setup( unsigned short polynominal, bool enable )
{
	using namespace stm32;
	spi_set_crc_polynomial( m_spi, polynominal );
	spi_calculate_crc( m_spi, enable );
	return 0;
}

/* Control CS manually*/
void spi_master::CS( bool val, int /*cs_no*/ )
{
	using namespace stm32;
	if( m_spi == SPI1 )
	{
		if( val ) gpio_set( spi1::SPI_PORT, spi1::SD_SPI_CS_PIN );
		else gpio_clr( spi1::SPI_PORT, spi1::SD_SPI_CS_PIN );
	}
	else if( m_spi == SPI2 )
	{
		//if( val ) gpio_set( spi2::SPI_PORT, spi2::SD_SPI_CS_PIN );
		//else gpio_clr( spi2::SPI_PORT, spi2::SD_SPI_CS_PIN );
	}
	else if( m_spi == SPI3 )
	{
		//if( val ) gpio_set( spi3::SPI_PORT, spi3::SD_SPI_CS_PIN );
		//else gpio_clr( spi3::SPI_PORT, spi3::SD_SPI_CS_PIN );
	}
}

/* Transfer data (nodma) */
uint16_t spi_master::transfer( uint16_t val )
{
	 using namespace stm32;
	if( !stm32::spi_is_enabled(m_spi) )
		return spi_device::err_noinit;
	 /*!< Wait until the transmit buffer is empty */
	 while(!spi_i2s_get_flag_status(m_spi, SPI_I2S_FLAG_TXE) )
	 {
		 //TODO: Slow down speed
		 // isix::isix_yield();
	 }
	 spi_i2s_send_data(m_spi, val );
	 while(!spi_i2s_get_flag_status(m_spi, SPI_I2S_FLAG_RXNE))
	 {
		//TODO: Slow down speed
		 // isix::isix_yield();
	 }
	 /*!< Return the byte read from the SPI bus */
	 return spi_i2s_receive_data(m_spi);
}

/* Disable enable the device */
void spi_master::enable( bool en )
{
	stm32::spi_cmd( m_spi, en );
}

} /* namespace drv */
} /* namespace stm32 */
