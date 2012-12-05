/*
 * mmc_slot.cpp
 *
 *  Created on: 05-12-2012
 *      Author: lucck
 */

#include "mmc/mmc_slot.hpp"
#include "mmc/mmc_card.hpp"
#include "mmc/mmc_error_codes.hpp"
#include <dbglog.h>

namespace drv {
namespace mmc {
/*--------------------------------------------------------------*/
namespace
{
	static const unsigned C_detect_interval = 200;
}
/*--------------------------------------------------------------*/
//Constructor
mmc_slot::mmc_slot( mmc_host &host, immc_det_pin &det_pin  )
	: m_det_pin(det_pin), m_host( host ), m_card(NULL),
	  m_det_timer(isix::isix_vtimer_create(det_card_insertion_raw_callback, this)),
	  m_card_init_req(true), m_p_card_inserted(0), m_card_sem(0, 1)
{
	if( m_det_timer )
	{
		isix::isix_vtimer_start_ms( m_det_timer, C_detect_interval );
	}
}
/*--------------------------------------------------------------*/
//Raw insertion handler
void mmc_slot::det_card_insertion_callback()
{
	bool card_inserted = m_det_pin.get();
	if( !m_p_card_inserted && card_inserted )
	{
		m_card_init_req = true;
		m_card_sem.signal_isr();
	}
	m_p_card_inserted = card_inserted;
}
/*--------------------------------------------------------------*/
//Destructor
mmc_slot::~mmc_slot()
{
	if( m_det_timer )
	{
		isix::isix_vtimer_stop( m_det_timer );
		isix::isix_vtimer_destroy( m_det_timer );
		m_det_timer = NULL;
	}
	delete m_card;
	m_card = 0;
}
/*--------------------------------------------------------------*/
//Get current allocated card
int mmc_slot::get_card( mmc_card* &card, int timeout )
{
	int ret = MMC_OK;
	if( m_det_pin.get() )
	{
		if( !m_card || m_card_init_req )
		{
			m_card_init_req = false;
			ret = mmc_card::detect( m_host, m_card );
			card = m_card;
		}
		return ret;
	}
	else
	{
		if( timeout < 0 )
			return MMC_CARD_NOT_PRESENT;
		ret = m_card_sem.wait( timeout );
		if( ret == isix::ISIX_ETIMEOUT )
			return MMC_CARD_NOT_PRESENT;
		else if( ret )
			return ret;
		ret = mmc_card::detect( m_host, m_card );
		card = m_card;
			return ret;
	}
}

/*--------------------------------------------------------------*/
} /* namespace drv */
} /* namespace stm32 */
