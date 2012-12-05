/*
 * mmc_slot.hpp
 *
 *  Created on: 05-12-2012
 *      Author: lucck
 */

#ifndef MMC_SLOT_HPP_
#define MMC_SLOT_HPP_

#include "noncopyable.hpp"
#include "mmc/immc_det_pin.hpp"
#include <isix.h>


namespace drv {
namespace mmc {

class mmc_host;
class mmc_card;

class mmc_slot: public fnd::noncopyable
{
public:
	mmc_slot( mmc_host &host, immc_det_pin &det_pin );
	~mmc_slot();
	int get_card( mmc_card* &card, int timeout = isix::ISIX_TIME_INFINITE );
private:
	//Raw insertion handler
	void det_card_insertion_callback();
	//Detect class insertion hander executed in interrupt content
	static void det_card_insertion_raw_callback( void* instance )
	{
		static_cast<mmc_slot*>(instance)->det_card_insertion_callback();
	}
private:
	//Wait for card semaphore
	immc_det_pin&				 	m_det_pin;
	mmc_host&						m_host;
	mmc_card*						m_card;
	isix::vtimer_struct* 			m_det_timer;
	volatile bool 					m_card_init_req;
	volatile bool 					m_p_card_inserted;
	isix::semaphore 				m_card_sem;
};

} /* namespace drv */
} /* namespace stm32 */
#endif /* MMC_SLOT_HPP_ */
