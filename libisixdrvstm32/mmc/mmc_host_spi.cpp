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
namespace mmc {
/*----------------------------------------------------------*/
namespace
{
	//SPI MODE
	const unsigned C_spi_mode = drv::spi_device::msb_first |
		drv::spi_device::data_8b | drv::spi_device::phase_1edge |
		drv::spi_device::cs_software | drv::spi_device::polar_cpol_low;
	//LOW clock freq
	const unsigned C_low_clk_khz_host = 400;
	//C block size
	const size_t C_block_len = 512;
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
	short mpos = 0;
	int ret = mmc_host::err_OK;
	qbuf[mpos++] = 0xff;
	qbuf[mpos++] = (req.get_op()|0x40) & 0x7f;
	qbuf[mpos++] = static_cast<uint8_t>(req.get_arg()>>24);
	qbuf[mpos++] = static_cast<uint8_t>(req.get_arg()>>16);
	qbuf[mpos++] = static_cast<uint8_t>(req.get_arg()>>8);
	qbuf[mpos++] = static_cast<uint8_t>(req.get_arg());
	qbuf[mpos++] = 0x95;		//CRC for first command only
	m_proc_cmd = req.get_op();
	CS(0);
	if( (ret=m_spi.write(qbuf, mpos, timeout)) )
	{
		m_proc_cmd = 0;
		return ret;
	}
	//FIXME temporary busy waiting
	uint8_t r1 = 0;
	if( req.get_flags() & mmc_command::resp_spi_s1 )
	{
		{
			//Skip data byte
			if( m_proc_cmd == mmc_command::OP_STOP_TRANSMISSION )
				m_spi.transfer(0xff);
			r1=0xff;
			for(int retry=0; retry<8 && r1==0xff; retry++)
				r1 = m_spi.transfer(0xff);
		}
		if( !(r1 & 0x80) )
		{
			//dbprintf("RRR1=%02x CMD=%02x", r1, req.get_op());
			req.set_resp_spi_r1( r1 );
		}
		else
		{
			dbprintf("Timeout error R1=0xFF");
			m_proc_cmd = 0;
			return mmc::MMC_CMD_RSP_TIMEOUT;
		}
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
	//Extra busy flag
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
		while(  r1 == 0x00 &&
				(isix::isix_get_jiffies()-t_start<timeout)
			  );
		if( prio >= 0 )
			isix::isix_task_change_prio( NULL, prio );
	}
	//Extra data CSD or CID for SD card simulation
	if( req.get_flags() & mmc_command::resp_spi_d16b )
	{
		dbprintf("EXTRA DATA ERR %d", req.get_err() );
		if( (ret=req.get_err()) ) return ret;
		ret = receive_data( req.get_resp_buffer(), 16, timeout );
		dbprintf("EXTRA DATA read ERR %d", ret );
		if( !ret )
			req.set_resp_status();
	}
	//Check chip select flags
	if( req.get_flags() & mmc_command::resp_spi_cs )
	{
		CS(1);
		m_proc_cmd = 0;
	}
	return ret;
}
/*----------------------------------------------------------*/
//Execute MMC data transfer
int mmc_host_spi::send_data( const void *buf, size_t len, unsigned timeout )
{
	//m_spi.transfer(0xFF);
	const char* bbuf = static_cast<const char*>(buf);
	for(size_t packet=0; packet<len; packet+=C_block_len)
	{
		if( m_proc_cmd == mmc_command::OP_WRITE_MULT_BLOCK )
			m_spi.transfer( MMC_STARTBLOCK_MWRITE );
		else
			m_spi.transfer( MMC_STARTBLOCK_WRITE );
		// Zapisz dane z bufora
		m_spi.write( bbuf+packet, len>C_block_len?C_block_len:len, timeout );
		// zapisz 16-bitowy CRC - nieistotny
		m_spi.transfer(0xFF);
		m_spi.transfer(0xFF);
		// Sprawdz token odpowiedzi
		uint8_t r1 = m_spi.transfer(0xFF);
		if( (r1&MMC_DR_MASK) != MMC_DR_ACCEPT)
		{
			m_spi.CS(true, 0);
			return MMC_DATA_NOT_ACCEPTED;
		}
		//TODO: TImeout Czekaj az karta bedzie wolna
		while( (r1=m_spi.transfer(0xff))==0 ) {}
	}
	if( m_proc_cmd == mmc_command::OP_WRITE_MULT_BLOCK )
	{
		dbprintf("STOP TRAN WRITE");
		m_spi.transfer( MMC_STOPTRAN_WRITE );
		//TODO: TImeout Czekaj az karta bedzie wolna
		uint8_t r1;
		while( (r1=m_spi.transfer(0xff))==0 ) {}
	}
	//Wait for free
	while( m_spi.transfer(0xff)==0 )
	{
	}
	// Zwolnij CS
	CS(1);
	// Zwroc OK
	return MMC_OK;
}
/*----------------------------------------------------------*/
	//Execute MMC data transfer
int mmc_host_spi::receive_data( void *buf, size_t len, unsigned timeout )
{
	char* bbuf = static_cast<char*>(buf);
	for(;;)
	{
		uint8_t r1 = m_spi.transfer(0xFF);
		if( r1 == MMC_STARTBLOCK_READ) break;
		else if((r1&MMC_DE_CHECK_MASK)==MMC_DE_ERROR)
		{
			m_spi.CS( true, 0 );
			dbprintf("Data token error 0x%02x", r1);
			//TODO fix it
			return MMC_DATA_NOT_ACCEPTED+1;
		}
	}
	for(size_t packet=0; packet<len; packet+=C_block_len)
	{
		m_spi.read( bbuf+packet, len>C_block_len?C_block_len:len, timeout );
		// Nie sprawdzaj CRC
		m_spi.flush(2);
		// Zwolnij CS
		// Wszystko OK
	}
	if( m_proc_cmd != mmc_command::OP_READ_MULT_BLOCK )
		m_spi.CS( true, 0 );
	//else
	//dbprintf("DONT DEASERT CS!!!");
	return MMC_OK;
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
		isix::isix_wait_ms( 5 );
		m_spi.set_mode( C_spi_mode, C_low_clk_khz_host );
		m_spi.CS( true, 0 );
		m_spi.flush( 10 );
		dbprintf("Power on");
		break;
	case mmc_host::ios_set_speed:
		if( m_spi_speed_limit_khz && param > m_spi_speed_limit_khz )
			param = m_spi_speed_limit_khz;
		m_spi.set_mode( C_spi_mode, param );
		break;
	case mmc_host::ios_set_bus_with:
		return err_not_supported;
	}
	return err_OK;
}
/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/
