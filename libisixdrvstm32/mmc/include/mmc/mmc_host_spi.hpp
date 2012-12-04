/*
 * mmc_host_spi.hpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef MMC_HOST_SPI_HPP_
#define MMC_HOST_SPI_HPP_
/*----------------------------------------------------------*/
#include "mmc/mmc_host.hpp"
#include "spi_device.hpp"
/*----------------------------------------------------------*/
namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
class mmc_host_spi : public mmc_host
{
public:
	//Constructor
	mmc_host_spi( immc_det_pin &det_gpio, spi_device &spi_dev )
		: mmc_host( det_gpio ), m_spi( spi_dev )
	{}
	//Execute MMC command
	virtual int execute_command( mmc_command &req, unsigned timeout );
	//Execute MMC data transfer
	virtual int send_data( const void *buf, size_t len, unsigned timeout );
	//Execute MMC data transfer
	virtual int receive_data( void *buf, size_t len, unsigned timeout );
	//Execute IO config
	virtual int set_ios( ios_cmd cmd, int param );
	//Get capabilities
	virtual unsigned get_capabilities() const
	{
		return mmc_host::cap_spi;
	}
private:
	void CS(bool en )
	{
		m_spi.CS( en, 0 );
	}
private:
	//SPI device
	spi_device &m_spi;
};
/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/
#endif /* MMC_HOST_SPI_HPP_ */
/*----------------------------------------------------------*/
