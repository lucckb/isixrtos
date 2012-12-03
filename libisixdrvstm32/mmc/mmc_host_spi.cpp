/*
 * mmc_host_spi.cpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */

/*----------------------------------------------------------*/
#include "mmc/mmc_host_spi.hpp"
#include "mmc/mmc_command.hpp"
#include "spi_device.hpp"
#include <dbglog.h>
/*----------------------------------------------------------*/
namespace drv {
/*----------------------------------------------------------*/
namespace
{
	static const unsigned C_spi_mode = drv::spi_device::msb_first |
		drv::spi_device::data_8b | drv::spi_device::phase_2edge |
		drv::spi_device::cs_software | drv::spi_device::polar_cpol_hi;
	static const unsigned C_low_clk_khz_host = 400;
}
/*----------------------------------------------------------*/
namespace		//Private tokens
{
	const uint8_t MMC_STARTBLOCK_READ		 =  0xFE;
	const uint8_t MMC_STARTBLOCK_WRITE		 =  0xFE;
	const uint8_t MMC_STARTBLOCK_MWRITE		 =  0xFC;
	const uint8_t MMC_STOPTRAN_WRITE		 =  0xFD;
	const uint8_t MMC_DE_MASK				 =  0x1F;
	const uint8_t MMC_DE_ERROR				 =  0x01;
	const uint8_t MMC_DE_CC_ERROR			 =  0x02;
	const uint8_t MMC_DE_ECC_FAIL			 =  0x04;
	const uint8_t MMC_DE_OUT_OF_RANGE		 =  0x04;
	const uint8_t MMC_DE_CARD_LOCKED		 =  0x04;
	const uint8_t MMC_DE_CHECK_MASK			 =  0xF1;
	const uint8_t MMC_DR_MASK				 =  0x1F;
	const uint8_t MMC_DR_ACCEPT				 =  0x05;
	const uint8_t MMC_DR_REJECT_CRC			 =  0x0B;
	const uint8_t MMC_DR_REJECT_WRITE_ERROR	 =  0x0D;
}
/*----------------------------------------------------------*/
//Execute MMC command
int mmc_host_spi::execute_command( mmc_command &req, unsigned timeout )
{
	uint8_t qbuf[8];
	int ret = mmc_host::err_OK;
	qbuf[0] = 0xff;
	qbuf[1] = req.get_op()|0x40;
	qbuf[2] = static_cast<uint8_t>(req.get_arg()>>24);
	qbuf[3] = static_cast<uint8_t>(req.get_arg()>>16);
	qbuf[4] = static_cast<uint8_t>(req.get_arg()>>8);
	qbuf[5] = static_cast<uint8_t>(req.get_arg());
	qbuf[6] = 0x95;		//CRC for first command only
	qbuf[7] = 0xff;		//Dummy byte
	CS(0);
	if( (ret=m_spi.write(qbuf, sizeof(qbuf), timeout)) )
		return ret;
	//FIXME temporary busy waiting
	uint8_t r1 = 0;
	if( req.get_flags() & mmc_command::resp_spi_s1 )
	{
		if ( req.get_flags()&mmc_command::resp_spi_busy )
		{
			isix::tick_t t_start = isix::isix_get_jiffies();
			timeout = isix::isix_ms2tick( timeout );
			//Downgrade priority during pool
			const int prio = isix::isix_task_change_prio( NULL, isix::isix_get_min_priority() );
			do
			{
				r1 = m_spi.transfer(0xff);
			}
			while(  r1 == 0xff &&
					(isix::isix_get_jiffies()-t_start<timeout)
				  );
			if( prio >= 0 )
				isix::isix_task_change_prio( NULL, prio );
		}
		else
		{
			r1 = m_spi.transfer(0xff);
		}
		if( r1 != 0xff )
			req.set_resp_spi_r1( r1 );
		else
			return mmc::MMC_CMD_RSP_TIMEOUT;
	}
	if( req.get_flags() & mmc_command::resp_spi_s2 )
	{
		req.set_resp_spi_r2( r1, m_spi.transfer(0xff) );
	}
	if( req.get_flags() & mmc_command::resp_spi_b4 )
	{
		req.set_resp_spi_b4(r1, m_spi.transfer(0xff),
			m_spi.transfer(0xff), m_spi.transfer(0xff), m_spi.transfer(0xff));
	}
	//Check chip select flags
	if( !(req.get_flags() & mmc_command::resp_spi_nocs) )
		CS(1);
	return ret;
}
/*----------------------------------------------------------*/
//Execute MMC data transfer
int mmc_host_spi::send_data( const void *buf, size_t len, unsigned timeout )
{
	// Zapisz dane z bufora
	m_spi.write( buf, len, timeout );
	// zapisz 16-bitowy CRC - nieistotny
	m_spi.transfer(0xFF);
	m_spi.transfer(0xFF);
	// Sprawdz token odpowiedzi
	uint8_t r1 = m_spi.transfer(0xFF);
	if( (r1&MMC_DR_MASK) != MMC_DR_ACCEPT)
	{
		m_spi.CS(true, 0);
		return r1;
	}
	// Czekaj az karta bedzie wolna
	while(!m_spi.transfer(0xFF));
	// Zwolnij CS
	CS(1);
	// Zwroc OK
	return 0;
}
/*----------------------------------------------------------*/
	//Execute MMC data transfer
int mmc_host_spi::receive_data( void *buf, size_t len, unsigned timeout )
{
	while(1)
	{
		uint8_t r1 = m_spi.transfer(0xFF);
		if( r1 == MMC_STARTBLOCK_READ)
			break;
		if((r1&MMC_DE_CHECK_MASK)==MMC_DE_ERROR)
		{
			m_spi.CS( true, 0 );
			return -(int)r1;
		}
	}
	m_spi.read( buf, len, timeout );
	// Nie sprawdzaj CRC
	m_spi.flush(2);
	// Zwolnij CS
	m_spi.CS( true, 0 );
	// Wszystko OK
	return 0;
}
/*----------------------------------------------------------*/
	//Execute IO config
int mmc_host_spi::set_ios( ios_cmd cmd, int param )
{
	switch( cmd )
	{
	case mmc_host::ios_pwr_off:
		m_spi.enable( false );
		break;
	case mmc_host::ios_pwr_on:
		m_spi.set_mode( C_spi_mode, C_low_clk_khz_host );
		m_spi.CS( true, 0 );
		m_spi.flush( 10 );
		dbprintf("Power on");
		break;
	case mmc_host::ios_set_speed:
		m_spi.set_mode( C_spi_mode, param );
		break;
	case mmc_host::ios_set_bus_with:
		return err_not_supported;
	}
	return err_OK;
}
/*----------------------------------------------------------*/
} /* namespace drv */

/*----------------------------------------------------------*/
