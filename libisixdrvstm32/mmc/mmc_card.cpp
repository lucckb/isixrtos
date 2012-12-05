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
#include <dbglog.h>
#include "mmc/mmc_host_spi.hpp"
#include "spi_device.hpp"
/*----------------------------------------------------------*/
namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
namespace {
	const unsigned C_card_timeout = 1000;
	const unsigned C_go_idle_retries = 2;
}
/*----------------------------------------------------------*/
//** Initialize the card on request
int mmc_card::detect()
{
	dbprintf("Initialize called ");
	m_host.set_ios( mmc_host::ios_pwr_on, 0 );
	int res = mmc_host::err_OK;
	unsigned retry = C_go_idle_retries;
	//Send GO IDLE state few times
	mmc_command cmd( mmc_command::OP_GO_IDLE_STATE, 0 );
	do
	{
	    res = m_host.execute_command_resp_check( cmd, C_card_timeout );
	}
	while( res != MMC_OK && --retry>0);
	if( res != MMC_OK) return MMC_CMD_RSP_TIMEOUT;
	dbprintf( "GO idle state %i", res );
	//Send IF cond
	cmd( mmc_command::OP_SEND_IF_COND, mmc_command::ARG_IFCOND_3V3_SUPPLY );
	if( (res=m_host.execute_command(cmd, C_card_timeout)) ) return res;
	if( cmd.get_err() == MMC_OK ) //SD card version 2.0
	{
		if( (res=cmd.validate_r7()) ) return res;
		dbprintf( "IF_COND state %i", cmd.get_err() );
		//Read OCR command
		cmd( mmc_command::OP_SDIO_READ_OCR, 0 );
		if( (res=m_host.execute_command(cmd, C_card_timeout)) )  return res;
		if( (res=cmd.get_err()) || (res=cmd.validate_r3()) ) return res;
		dbprintf( "READ_OCR state %i %08lx", cmd.get_err(), cmd.get() );
		//Downgrade the task priority (card pooling)
		int pprio = isix::isix_task_change_prio(NULL, isix::isix_get_min_priority());
		if( !pprio ) return pprio;
		//Wait for card will be ready and initialized
		static const int Cond_retries = 100;
		for(int retry=0; retry<Cond_retries; retry++ )
		{
			cmd( mmc_command::OP_APP_CMD, 0 );
			if( (res=m_host.execute_command_resp_check( cmd, C_card_timeout ) ) ) break;
			cmd( mmc_command::OP_SD_APP_OP_COND,  mmc_command::ARG_OPCOND_HCS );
			if( (res=m_host.execute_command_resp_check(cmd,C_card_timeout)) ) break;
			if( cmd.get_card_state()!=mmc_command::card_state_IDLE ) break;
			isix::isix_wait_ms(10);
		}
		//Restore isix prio
		if( (pprio = isix::isix_task_change_prio(NULL, pprio))<0 ) return pprio;
		if( cmd.get_err() ) return cmd.get_err();
		if(cmd.get_card_state()==mmc_command::card_state_IDLE) return MMC_CMD_RSP_TIMEOUT;
		dbprintf("OP_COND code %i", cmd.get_err());
		//Read OCR check HI capacity card
		cmd( mmc_command::OP_SDIO_READ_OCR, 0 );
		if( (res = m_host.execute_command_resp_check(cmd, C_card_timeout)) )
		    return res;
		if( cmd.get_r3_ccs() ) m_type = type_sdhc;
		else m_type = type_sd_v2;
		dbprintf("CCS bit=%d", cmd.get_r3_ccs());
	}
	else if( cmd.get_err() == MMC_ILLEGAL_CMD )	//Not version 2
	{
		//Check the type of the SD card
		cmd( mmc_command::OP_APP_CMD, 0 );
		if( (res=m_host.execute_command( cmd, C_card_timeout ) ) )
			return res;
		if( cmd.get_err() == MMC_OK )	//SD card
		{
			//Downgrade the task priority (card pooling)
			int pprio = isix::isix_task_change_prio(NULL, isix::isix_get_min_priority());
			if( !pprio ) return pprio;
			//WAIT for SDV1 initialization
			static const int Cond_retries = 100;
			for(int retry=0; retry<Cond_retries; retry++ )
			{
				cmd( mmc_command::OP_SD_APP_OP_COND,  0 );
				if( (res=m_host.execute_command_resp_check(cmd,C_card_timeout)) ) break;
				if( cmd.get_card_state()!=mmc_command::card_state_IDLE ) break;
				isix::isix_wait_ms(10);
				cmd( mmc_command::OP_APP_CMD, 0 );
				if( (res=m_host.execute_command_resp_check(cmd, C_card_timeout))) break;
			}
			//Restore isix prio
			if( (pprio = isix::isix_task_change_prio(NULL, pprio))<0 ) return pprio;
			if( cmd.get_err() ) return cmd.get_err();
			if( cmd.get_card_state()==mmc_command::card_state_IDLE ) return MMC_CMD_RSP_TIMEOUT;
			dbprintf("OP_COND code %i", cmd.get_err());
			m_type = type_sd_v1;
		}
		else if( cmd.get_err() == MMC_ILLEGAL_CMD )	//MMC card
		{
			//Downgrade the task priority (card pooling)
			int pprio = isix::isix_task_change_prio(NULL, isix::isix_get_min_priority());
			if( !pprio ) return pprio;
			//WAIT for MMC initialization
			static const int Cond_retries = 100;
			for(int retry=0; retry<Cond_retries; retry++ )
			{
				cmd( mmc_command::OP_SEND_OP_COND,  0 );
				if( (res=m_host.execute_command_resp_check(cmd, C_card_timeout)) )break;
			}
			//Restore isix prio
			if( (pprio = isix::isix_task_change_prio(NULL, pprio))<0 )
				return pprio;
			if( cmd.get_err() ) return cmd.get_err();
			if( cmd.get_card_state()==mmc_command::card_state_IDLE ) return MMC_CMD_RSP_TIMEOUT;
			m_type = type_mmc;
		}
	}
	//Continue
	return res;
}
/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/
