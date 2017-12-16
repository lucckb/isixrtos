/*
 * mmc_slot.cpp
 *
 *  Created on: 05-12-2012
 *      Author: lucck
 */

#include "mmc/mmc_slot.hpp"
#include "mmc/mmc_card.hpp"
#include "mmc/mmc_error_codes.hpp"
#include <foundation/sys/dbglog.h>

namespace drv {
namespace mmc {
/*--------------------------------------------------------------*/
namespace
{
	static const unsigned C_detect_interval = 500;
}
/*--------------------------------------------------------------*/
//Constructor
mmc_slot::mmc_slot( mmc_host &host, immc_det_pin &det_pin  )
	: m_det_pin(det_pin), m_host( host ), m_card(NULL),
	  m_det_timer(isix_vtimer_create()),
	  m_event(0), m_p_card_inserted(0), m_init_req(true), m_card_sem(0, 1)
{
	if( m_det_timer )
	{
		isix_vtimer_start( m_det_timer, det_card_insertion_raw_callback,
				this, isix::ms2tick(C_detect_interval), true );
	}
}
/*--------------------------------------------------------------*/
//Raw insertion handler
void mmc_slot::det_card_insertion_callback()
{
	bool inserted = m_det_pin.get();
	if( !m_p_card_inserted && inserted ) {
		m_event = card_inserted;
		m_init_req = true;
		m_card_sem.signal_isr();
		if( m_callback ) {
			m_callback( card_inserted );
		}
	}
	else if( m_p_card_inserted && !inserted ) {
		m_event = card_removed;
		m_card_sem.signal_isr();
		if( m_callback ) {
			m_callback( card_removed );
		}
	}
	m_p_card_inserted = inserted;
}
/*--------------------------------------------------------------*/
//Destructor
mmc_slot::~mmc_slot()
{
	if( m_det_timer ) {
		isix_vtimer_cancel( m_det_timer );
		isix_vtimer_destroy( m_det_timer );
		m_det_timer = NULL;
	}
	delete m_card;
	m_card = 0;
}
/*--------------------------------------------------------------*/
//Get current allocated card
int mmc_slot::get_card( mmc_card* &card )
{
	int ret = MMC_CARD_NOT_PRESENT;
	if( m_det_pin.get() ) {
		if( !m_card  || m_init_req ) {
			ret = mmc_card::detect( m_host, m_card );
			m_init_req = false;
			if(!ret) card = m_card;
		} else {
			ret = MMC_OK;
			card = m_card;
		}
	}
	return ret;
}

/*--------------------------------------------------------------*/
//Wait for change status
int mmc_slot::check( int timeout )
{
	if( timeout < 0 ) {
		return m_det_pin.get()?card_inserted:card_removed;
	} else {
		int ret = m_card_sem.wait( timeout );
		if( ret ) return ret;
		ret = m_event;
		return ret;
	}
}
/*--------------------------------------------------------------*/
} /* namespace drv */
} /* namespace stm32 */
