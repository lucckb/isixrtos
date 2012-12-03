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
/*----------------------------------------------------------*/
namespace {
	const unsigned C_card_timeout = 1000;
	const unsigned C_go_idle_retries = 2;
}

/*----------------------------------------------------------*/
//** Initialize the card on request
int mmc_card::initialize()
{
	dbprintf("Initialize called ");
	m_host.set_ios( mmc_host::ios_pwr_on, 0 );
	int res = mmc_host::err_OK;
	unsigned retry = C_go_idle_retries;
	//Send GO IDLE state few times
	mmc_command cmd( mmc_command::OP_GO_IDLE_STATE, 0 );
	do
	{
		if( (res=m_host.execute_command( cmd, C_card_timeout ) ) )
		{
			return res;
		}
		res = cmd.get_err();
	}
	while( res != mmc::MMC_IN_IDLE_STATE && --retry>0);
	if( res != mmc::MMC_IN_IDLE_STATE)
		return mmc::MMC_CMD_RSP_TIMEOUT;

	dbprintf( "GO idle state %i", res );
	//Send IF cond
	cmd( mmc_command::OP_SEND_IF_COND, 0x00000122 );
	if( (res=m_host.execute_command( cmd, C_card_timeout ) ) )
		return res;
	dbprintf( "IF_COND state %i", cmd.get_err() );
	return res;
}
/*----------------------------------------------------------*/
} /* namespace drv */

/*----------------------------------------------------------*/
