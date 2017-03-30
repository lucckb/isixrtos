/*
 * mmc_card.cpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#include "mmc/mmc_card.hpp"
#include "mmc/mmc_host.hpp"
#include "mmc/mmc_command.hpp"
#include "mmc/mmc_host_spi.hpp"
#include "spi_device.hpp"
#include <new>

/*----------------------------------------------------------*/
#ifdef DEBUG_MMC_MMC_CARD_CPP
#include <dbglog.h>
#else
#define dbprintf(...) do {} while(0)
#endif
/*----------------------------------------------------------*/
namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
namespace {
	const unsigned C_card_timeout = 2000;
	const unsigned C_go_idle_retries = 10;
}

/*----------------------------------------------------------*/
/* Constructor */
mmc_card::mmc_card( mmc_host &host, card_type type )
  : m_host(host) , m_type(type), m_error(0), m_rca(0),
    m_block_count_avail(false), m_bus_width(mmc_host::bus_width_1b),
    m_nr_sectors(0)
{
	if( !m_host.is_spi() )
	{
		m_error = sd_mode_initialize();
		dbprintf("SD mode init code %i", m_error);
	}
	else
	{
		mmc_command cmd( mmc_command::OP_CRC_ON_OFF, 0 );
		m_error = m_host.execute_command_resp_check( cmd, C_card_timeout );
		dbprintf("CRCON_OFF ret=%i", m_error );
	}
	if( !m_error )
	{
		mmc_command cmd( mmc_command::OP_SET_BLOCKLEN, C_sector_size );
		m_error = m_host.execute_command_resp_check( cmd, C_card_timeout );
		dbprintf("BLOCKLEN ret=%i", m_error );
	}
    if ( !m_host.is_spi() && m_type != type_mmc )
    {
        m_error = read_scr_card_info();
        dbprintf("read_scr_card_info %i", m_error );
    }
    uint32_t tran_speed {};
    if( !m_error )
    {
		//Update card parameters
		tran_speed = read_csd_card_info();
		dbprintf("read_csd_card_info %i", m_error );
    }
	if( !m_error )
	{
		dbprintf("SET tran speed to %lu ", tran_speed );
		//Set maximum speed
		m_error = m_host.set_ios( mmc_host::ios_set_speed, tran_speed/1000 );
	}
    if( !m_host.is_spi() && m_type!=type_mmc && !m_error )
    {
   		if( (m_host.get_capabilities()&mmc_host::cap_4bit) &&
            (m_bus_width >= mmc_host::bus_width_4b) )
        {
            m_error = sd_enable_wide_bus( mmc_host::bus_width_4b );
            dbprintf("Try to enable wide bus4b OP %i", m_error );
        }
   		else
   		{
   			m_error = sd_enable_wide_bus( mmc_host::bus_width_1b );
   			dbprintf("Try to enable wide bus1b OP %i", m_error );
   		}
   		if( !m_error )
   		{
   			m_error = m_host.set_ios( mmc_host::ios_set_bus_with, m_bus_width);
   			dbprintf("Switch SD bus to %i mode", m_error );
   		}
    }
}
/*----------------------------------------------------------*/
//Read extended card info
int mmc_card::read_scr_card_info()
{
    int ret = MMC_OK;
    mmc_command cmd;
    scr scr;
    static const size_t C_scr_size = 8;
    do {
    	cmd( mmc_command::OP_APP_CMD, unsigned(m_rca)<<16 );
    	if(( ret=m_host.execute_command_resp_check( cmd, C_card_timeout ))) break;
    	cmd( mmc_command::OP_SD_APP_SEND_SCR, 0 );
     	if( (ret=m_host.receive_data_prep(cmd.get_resp_buffer(2), C_scr_size , C_card_timeout ))) break;
    	if(( ret=m_host.execute_command_resp_check( cmd, C_card_timeout ))) break;
       	if( (ret=m_host.receive_data( cmd.get_resp_buffer(2), C_scr_size, C_card_timeout ))) break;
        if( (ret=cmd.decode_scr(scr)) ) break;
    } while(0);
    if( !ret )
    {
        m_block_count_avail = scr.is_set_block_count;
        m_bus_width = (scr.bus_width_4b)?(mmc_host::bus_width_4b):(mmc_host::bus_width_1b);
        if( !scr.bus_width_1b )
            ret = MMC_UNRECOGNIZED_SCR;
    }
    return ret;
}
/*----------------------------------------------------------*/
//Enable wide bus operation
int mmc_card::sd_enable_wide_bus( int width )
{
    int ret = MMC_OK;
    mmc_command cmd;
    do
    {
        cmd( mmc_command::OP_APP_CMD, unsigned(m_rca)<<16 );
    	if(( ret=m_host.execute_command_resp_check( cmd, C_card_timeout ))) break;
    	cmd( mmc_command::OP_APP_SD_SET_BUSWIDTH, (width==mmc_host::bus_width_4b)?(0x02):(0x00) );
    	if(( ret=m_host.execute_command_resp_check( cmd, C_card_timeout ))) break;
    }
    while(0);
    return ret;
}
/*----------------------------------------------------------*/
//** Initialize the card on request
int mmc_card::probe( mmc_host &host, mmc_card::card_type &type )
{
	int res = mmc_host::err_OK;
	unsigned retry = C_go_idle_retries;
	host.set_ios( mmc_host::ios_pwr_on, 0 );
	//Send GO IDLE state few times
	mmc_command cmd( mmc_command::OP_GO_IDLE_STATE, 0 );
	do
	{
	    res = host.execute_command_resp_check( cmd, C_card_timeout );
	    dbprintf("IDLE_STATE %u", res );
	}
	while( res != MMC_OK && --retry>0);
	if( res != MMC_OK) return MMC_CMD_RSP_TIMEOUT;
	dbprintf( "GO idle state %i", res );
	//Send IF cond
	cmd( mmc_command::OP_SEND_IF_COND, mmc_command::ARG_IFCOND_3V3_SUPPLY );
	res = host.execute_command(cmd, C_card_timeout);
	dbprintf( "OP_SEND_IF_COND %i %i", res, cmd.get_err() );
	if(!res && cmd.get_err() == MMC_OK ) //SD card version 2.0
	{
		if( (res=cmd.validate_r7()) ) return res;
		dbprintf( "IF_COND state %i", cmd.get_err() );
		//Read OCR command
		if( host.is_spi() )
		{
			cmd( mmc_command::OP_SDIO_READ_OCR, 0 );
			if( (res=host.execute_command(cmd, C_card_timeout)) )  return res;
			if( (res=cmd.get_err()) || (res=cmd.validate_r3()) ) return res;
		}
		//Downgrade the task priority (card pooling)
		int pprio = isix_task_change_prio(NULL, isix_get_min_priority());
		if( !pprio ) return pprio;
		//Wait for card will be ready and initialized
		static const int Cond_retries = 100;
		for(int retry=0; retry<Cond_retries; retry++ )
		{
			cmd( mmc_command::OP_APP_CMD, 0 );
			if( (res=host.execute_command_resp_check( cmd, C_card_timeout ) ) ) break;
			cmd( mmc_command::OP_SD_APP_OP_COND,
				mmc_command::ARG_OPCOND_HCS|(host.is_spi()?0:mmc_command::ARG_OPCOND_VOLT_ALL) );
			if( (res=host.execute_command_resp_check(cmd,C_card_timeout)) ) break;
			if( cmd.get_card_state()!=mmc_command::card_state_IDLE ) break;
			isix_wait_ms(10);
		}
		//Restore isix prio
		if( (pprio = isix_task_change_prio(NULL, pprio))<0 ) return pprio;
		if( cmd.get_err() ) return cmd.get_err();
		if(cmd.get_card_state()==mmc_command::card_state_IDLE) return MMC_CMD_RSP_TIMEOUT;
		dbprintf("OP_COND code %i", cmd.get_err());
		//Read OCR check HI capacity card
		if( host.is_spi() )
		{
			cmd( mmc_command::OP_SDIO_READ_OCR, 0 );
			if( (res = host.execute_command_resp_check(cmd, C_card_timeout)) )
				return res;
		}
		if( cmd.get_r3_ccs() ) type = type_sdhc;
		else type = type_sd_v2;
		dbprintf("CCS bit=%d", cmd.get_r3_ccs());
	}
	else if( cmd.get_err() != MMC_OK )	//Not version 2
	{
		dbprintf("Type 1 card");
		//Check the type of the SD card
		cmd( mmc_command::OP_APP_CMD, 0 );
		res = host.execute_command( cmd, C_card_timeout );
		if( !res && ((host.is_spi()&&cmd.get_err()==MMC_OK) ||
		  (!host.is_spi() && cmd.get_err()==MMC_ILLEGAL_CMD))  )	//SD card
		{
			//Downgrade the task priority (card pooling)
			int pprio = isix_task_change_prio(NULL, isix_get_min_priority());
			if( !pprio ) return pprio;
		    //WAIT for SDV1 initialization
			static const int Cond_retries = 100;
			for(int retry=0; retry<Cond_retries; retry++ )
			{
				cmd( mmc_command::OP_SD_APP_OP_COND, host.is_spi()?0:mmc_command::ARG_OPCOND_VOLT_ALL );
				if( (res=host.execute_command_resp_check(cmd,C_card_timeout)) ) break;
				if( cmd.get_card_state()!=mmc_command::card_state_IDLE ) break;
				isix_wait_ms(10);
				cmd( mmc_command::OP_APP_CMD, 0 );
				if( (res=host.execute_command_resp_check(cmd, C_card_timeout))) break;
			}
			//Restore isix prio
			if( (pprio = isix_task_change_prio(NULL, pprio))<0 ) return pprio;
			if( cmd.get_err() ) return cmd.get_err();
			if( cmd.get_card_state()==mmc_command::card_state_IDLE ) return MMC_CMD_RSP_TIMEOUT;
			dbprintf("OP_COND code %i", cmd.get_err());
			type = type_sd_v1;
		}
		else	//MMC card
		{
			//Downgrade the task priority (card pooling)
			int pprio = isix_task_change_prio(NULL, isix_get_min_priority());
			if( !pprio ) return pprio;
			//WAIT for MMC initialization
			static const int Cond_retries = 100;
			cmd( mmc_command::OP_SEND_OP_COND,  host.is_spi()?0:mmc_command::ARG_OPCOND_VOLT_ALL );
			for(int retry=0; retry<Cond_retries; retry++ )
			{
				if( (res=host.execute_command_resp_check(cmd,C_card_timeout)) ) break;
				if( cmd.get_card_state()!=mmc_command::card_state_IDLE ) break;
				isix_wait_ms(10);
			}
			//Restore isix prio
			if( (pprio = isix_task_change_prio(NULL, pprio))<0 )
				return pprio;
			if( cmd.get_err() ) return cmd.get_err();
			if( cmd.get_card_state()==mmc_command::card_state_IDLE ) return MMC_CMD_RSP_TIMEOUT;
			type = type_mmc;
		}
	}
	return res;
}

/*----------------------------------------------------------*/
int mmc_card::detect( mmc_host &host, mmc_card* &old_card )
{
	card_type ctype(type_none);
	int ret = probe( host, ctype );
	if( ret ) return ret;
	if(!old_card )
		old_card = new mmc_card( host, ctype );
	/* TODO: In the feature delete old card if type mismatch for
	 * example previous was SDIO but now MMC_SD card*/
	else
	{
		//Placement new
		old_card->~mmc_card();
		old_card = new(old_card) mmc_card( host, ctype );
		ret = old_card->get_error();
	}
	return ret;
}
/*----------------------------------------------------------*/
//initialize card in SD mode
int mmc_card::sd_mode_initialize()
{
	mmc_command cmd;
    int ret;
    do
    {
        //send all cid
        cmd( mmc_command::OP_ALL_SEND_CID, 0 );
        if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout)) ) break;
        dbprintf("OP_ALL_SEND_CID [%i]", ret );
        //Ask relative addr
        if( m_type != type_mmc)
        {
			cmd( mmc_command::OP_SET_REL_ADDR, 0 );
			if( (ret=m_host.execute_command(cmd, C_card_timeout)) ) break;
			dbprintf("OP_SET_REL_ADDR [%i]", ret );
			if( (ret=cmd.validate_r6(m_rca)) ) break;
			dbprintf("VALIDATE_R6 [%i] RCA [%04x]", ret, m_rca);
        }
        else //MMC push relative addr
        {
        	m_rca = 1;
        	cmd( mmc_command::OP_SET_REL_ADDR, unsigned(m_rca)<<16 );
        	if( (ret=m_host.execute_command(cmd, C_card_timeout)) ) break;
        }
        //Select deselect card
        cmd( mmc_command::OP_SEL_DESEL_CARD, unsigned(m_rca)<<16 );
        if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout)) ) break;
        dbprintf("OP_SEL_DESEL_CARD [%i]", ret );
    } while(0);
    return ret;
}
/*----------------------------------------------------------*/
//Update card parameters
uint32_t mmc_card::read_csd_card_info()
{
	uint32_t tran_speed {};
	mmc_command cmd;
	do
	{
		if(!m_host.is_spi())
		{
			//Select deselect card
			cmd( mmc_command::OP_SEL_DESEL_CARD, 0 );
			if( (m_error=m_host.execute_command(cmd, C_card_timeout)) ) break;
			dbprintf("OP_SEL_DESEL_CARD [%i]", m_error );
		}
		cmd( mmc_command::OP_SEND_CSD, unsigned(m_rca)<<16 );
		if( (m_error=m_host.execute_command(cmd, C_card_timeout))) break;
		dbprintf("OP_SEND_CSD [%i]", m_error );
		if( (m_error=cmd.decode_csd_sectors(m_nr_sectors, m_type==type_mmc) ) ) break;
		if( (m_error=cmd.decode_csd_tran_speed(tran_speed)) ) break;
		if(!m_host.is_spi())
		{
			//Select deselect card
			cmd( mmc_command::OP_SEL_DESEL_CARD, unsigned(m_rca)<<16 );
			if( (m_error=m_host.execute_command_resp_check(cmd, C_card_timeout)) ) break;
			dbprintf("OP_SEL_DESEL_CARD [%i]", m_error );
		}
	} while(0);
	return tran_speed;
}
/*----------------------------------------------------------*/
/* Write multi sectors */
int mmc_card::write_multi_blocks( const void* buf, unsigned long laddr,  std::size_t count )
{
	int ret, ret2;
	mmc_command cmd;
	do
	{
		if( m_block_count_avail )
		{
			/* Set Block Size for Card */
			cmd( mmc_command::OP_APP_CMD, unsigned(m_rca)<<16 );
			if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
			cmd( mmc_command::OP_SET_BLOCK_COUNT, count );
			if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
		}
		cmd( mmc_command::OP_WRITE_MULT_BLOCK, laddr );
		if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
		if( (ret=m_host.send_data( buf, C_sector_size*count, C_card_timeout ))) break;
	} while(0);
	do
	{
		if( !m_block_count_avail && !m_host.is_spi() )
		{
			cmd( mmc_command::OP_STOP_TRANSMISSION, 0 );
			if( (ret2=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
		}
		if((ret2 = m_host.wait_data_ready( C_card_timeout ))) break;
	} while(0);
	if( !ret ) ret = ret2;
	return ret;
}
/*----------------------------------------------------------*/
/* Read multi sectors */
int mmc_card::read_multi_blocks( void* buf, unsigned long laddr, std::size_t count )
{
	int ret;
	mmc_command cmd;
	do
	{
		if( m_block_count_avail )
		{
			/* Set Block Size for Card */
			cmd( mmc_command::OP_APP_CMD, unsigned(m_rca)<<16 );
			if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
			cmd( mmc_command::OP_SET_BLOCK_COUNT, count );
			if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
		}
		cmd( mmc_command::OP_READ_MULT_BLOCK, laddr);
		if( (ret=m_host.receive_data_prep( buf, C_sector_size*count, C_card_timeout ))) break;
		if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
		if( (ret=m_host.receive_data( buf, C_sector_size*count, C_card_timeout ))) break;
	}
	while(0);
	//Always send stop command after the error also
	if( !m_block_count_avail )
	{
		cmd( mmc_command::OP_STOP_TRANSMISSION, 0 );
		const int rstop = m_host.execute_command_resp_check(cmd, C_card_timeout);
		if(!ret) ret = rstop;
	}
	return ret;
}

/*----------------------------------------------------------*/
/* Write single block */
int mmc_card::write_single_block( const void* buf, unsigned long laddr )
{
	int ret;
	mmc_command cmd( mmc_command::OP_WRITE_SINGLE_BLOCK, laddr );
	do
	{
		if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
		if( (ret=m_host.send_data( buf, C_sector_size, C_card_timeout ))) break;
		if((ret = m_host.wait_data_ready( C_card_timeout ))) break;
	} while(0);
	return ret;
}
/*----------------------------------------------------------*/
/* Read single block */
int mmc_card::read_single_block( void* buf, unsigned long laddr )
{
	int ret;
	mmc_command cmd( mmc_command::OP_READ_SINGLE_BLOCK, laddr );
	do
	{
		if( (ret=m_host.receive_data_prep( buf, C_sector_size, C_card_timeout ))) break;
		if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
		if( (ret=m_host.receive_data( buf, C_sector_size, C_card_timeout ))) break;
	} while(0);
	return ret;
}
/*----------------------------------------------------------*/
/** Write the block */
int mmc_card::write( const void* buf, unsigned long sector, std::size_t count )
{
	if( m_error ) return m_error;
	if( m_type != type_sdhc ) sector *= C_sector_size;
	if( count == 1 )
		return write_single_block( buf, sector );
	else
		return write_multi_blocks( buf, sector, count );
}
/*----------------------------------------------------------*/
/** Read the block */
int mmc_card::read ( void* buf, unsigned long sector, std::size_t count )
{
	if( m_error ) return m_error;
	if( m_type != type_sdhc ) sector *= C_sector_size;
	if( count == 1 )
		return read_single_block( buf, sector );
	else
		return read_multi_blocks( buf, sector, count );
}
/*----------------------------------------------------------*/
/* Get card CID */
int mmc_card::get_cid( cid &c ) const
{
	int ret;
	mmc_command cmd;
	if( m_error ) return m_error;
	do
	{
		if(!m_host.is_spi())
		{
			//Select deselect card
			cmd( mmc_command::OP_SEL_DESEL_CARD, 0 );
			if( (ret=m_host.execute_command(cmd, C_card_timeout)) ) break;
			dbprintf("OP_SEL_DESEL_CARD [%i]", m_error );
		}
		cmd( mmc_command::OP_SEND_CID, unsigned(m_rca)<<16 );
		if( (ret=m_host.execute_command(cmd, C_card_timeout))) break;
		if( (ret=cmd.decode_cid( c,m_type==type_mmc )) ) break;
		if(!m_host.is_spi())
		{
			//Select deselect card
			cmd( mmc_command::OP_SEL_DESEL_CARD, unsigned(m_rca)<<16 );
			if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout)) ) break;
			dbprintf("OP_SEL_DESEL_CARD [%i]", m_error );
		}
	} while(0);
	dbprintf("DECODE CID %i", ret);
	return ret;
}

/*----------------------------------------------------------*/
/* Get erase size */
int mmc_card::get_erase_size(uint32_t &sectors) const
{
	int ret;
	mmc_command cmd;
	if( m_error ) return m_error;
	//Decode from CSD
	if( m_type == type_mmc || m_type == type_sd_v1  )
	{
		do {
			if(!m_host.is_spi())
			{
				//Select deselect card
				cmd( mmc_command::OP_SEL_DESEL_CARD, 0 );
				if( (ret=m_host.execute_command(cmd, C_card_timeout)) ) break;
				dbprintf("OP_SEL_DESEL_CARD [%i]", m_error );
			}
			cmd( mmc_command::OP_SEND_CSD, unsigned(m_rca)<<16 );
			if( (ret=m_host.execute_command(cmd, C_card_timeout))) break;
			if( (ret=cmd.decode_csd_erase(sectors,m_type==type_mmc)) ) break;
			if(!m_host.is_spi())
			{
				//Select deselect card
				cmd( mmc_command::OP_SEL_DESEL_CARD, unsigned(m_rca)<<16 );
				if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout)) ) break;
				dbprintf("OP_SEL_DESEL_CARD [%i]", m_error );
			}
		} while(0);
	}
	else
	{
		uint32_t sdbuf[16];
		do {
			cmd( mmc_command::OP_APP_CMD, unsigned(m_rca) << 16 );
			if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
			cmd( mmc_command::OP_SD_APP_STATUS, unsigned(m_rca) << 16 );
			if( (ret=m_host.receive_data_prep( sdbuf, sizeof(sdbuf), C_card_timeout ))) break;
			if( (ret=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
			if( (ret=m_host.receive_data( sdbuf, sizeof(sdbuf), C_card_timeout ))) break;
		} while(0);
		if( !ret )
			sectors = mmc_command::decode_sdstat_erase( sdbuf );
	}
	return ret;
}


/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/
