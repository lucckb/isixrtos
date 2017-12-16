/*
 * mmc_slot.hpp
 *
 *  Created on: 05-12-2012
 *      Author: lucck
 */

#ifndef MMC_SLOT_HPP_
#define MMC_SLOT_HPP_

#include <foundation/algo/noncopyable.hpp>
#include "mmc/immc_det_pin.hpp"
#include "mmc/mmc_error_codes.hpp"
#include <isix.h>
#include <functional>

namespace drv {
namespace mmc {

class mmc_host;
class mmc_card;

class mmc_slot: public fnd::noncopyable {
public:
	//! Card status
	enum status {
		card_removed =  0x1,
		card_inserted = 0x2
	};
	static const int C_no_block = -1;
	using callback_t = std::function<void(status)>;
public:
	//Constructor
	mmc_slot( mmc_host &host, immc_det_pin &det_pin );
	//Destructor
	~mmc_slot();
	//Get card status
	int get_card( mmc_card* &card );
	//Wait for change status
	int check( int timeout = ISIX_TIME_INFINITE );
	//Register notification callback
	void connect( callback_t cb ) {
		m_callback = cb;
	}
private:
	//Raw insertion handler
	void det_card_insertion_callback();
	//Detect class insertion hander executed in interrupt content
	static void det_card_insertion_raw_callback( void* instance ) {
		static_cast<mmc_slot*>(instance)->det_card_insertion_callback();
	}
private:
	//Wait for card semaphore
	immc_det_pin&					m_det_pin;
	mmc_host&						m_host;
	mmc_card*						m_card;
	isix_vtimer*					m_det_timer;
	volatile uint8_t				m_event;
	volatile bool					m_p_card_inserted;
	volatile bool					m_init_req;
	isix::semaphore					m_card_sem;
	callback_t						m_callback;
};

} /* namespace drv */
} /* namespace stm32 */
#endif /* MMC_SLOT_HPP_ */
