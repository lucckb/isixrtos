/*
 * mmc_host.cpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */
/*--------------------------------------------------------------*/
#include "mmc/mmc_host.hpp"
#include "mmc/mmc_command.hpp"
#include "mmc/immc_det_pin.hpp"
#include "mmc/mmc_card.hpp"
#include <isix.h>
#include <dbglog.h>
/*--------------------------------------------------------------*/
namespace drv {
namespace mmc {
/*--------------------------------------------------------------*/
namespace
{
	static const unsigned C_detect_interval = 200;
}
/*--------------------------------------------------------------*/
//Constructor
mmc_host::mmc_host( immc_det_pin &detect_pin )
	:  m_card(new mmc_card(*this)), m_det_pin(detect_pin),
	  m_det_timer(isix::isix_vtimer_create(det_card_insertion_raw_callback, this)),
	  m_card_init_req(m_det_pin.get()), m_p_card_inserted(0),
	  m_card_sem(0, 1)
{
	if( m_det_timer )
	{
		isix::isix_vtimer_start_ms( m_det_timer, C_detect_interval );
	}
}
/*--------------------------------------------------------------*/
//Raw insertion handler
void mmc_host::det_card_insertion_callback()
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
mmc_host::~mmc_host()
{
	if( m_det_timer )
	{
		isix::isix_vtimer_stop( m_det_timer );
		isix::isix_vtimer_destroy( m_det_timer );
		m_det_timer = NULL;
	}
	delete m_card;
}
/*--------------------------------------------------------------*/
//Get current allocated card
mmc_card* mmc_host::get_card( int timeout )
{
	if( get_cd() )
	{
		if( m_card_init_req )
		{
			m_card_init_req = false;
			dbprintf("Initialize code=%i",m_card->initialize());
		}
		return m_card;
	}
	else
	{
		if( timeout < 0) return NULL;
		else
		{
			m_card_sem.wait( timeout );
			if( m_card_init_req )
			{
				m_card_init_req = false;
				dbprintf("Initialize code #2=%i", m_card->initialize());
			}
			return m_card;
		}
	}
}

/*--------------------------------------------------------------*/
} /* namespace drv */
}
