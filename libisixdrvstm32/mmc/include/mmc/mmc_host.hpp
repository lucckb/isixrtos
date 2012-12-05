/*
 * mmc_host.hpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */

/*----------------------------------------------------------*/
#ifndef MMC_HOST_HPP_
#define MMC_HOST_HPP_
/*----------------------------------------------------------*/
#include <noncopyable.hpp>
#include <cstddef>
#include <isix.h>
#include "mmc/immc_det_pin.hpp"
#include "mmc/mmc_command.hpp"
/*----------------------------------------------------------*/
namespace isix
{
	struct vtimer_struct;
}
/*----------------------------------------------------------*/
namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
class mmc_card;

/*----------------------------------------------------------*/
/* MMC host controler */
class mmc_host : private fnd::noncopyable
{
public:
	//Error code
	enum err
	{
		err_OK,
		err_not_supported=1000
	};
	//!Host capabilities
	enum mmc_host_cap
	{
		cap_pwroff  	= 1 << 0,		//! Capable cutoff power
		cap_spi  		= 1 << 1,		//! Host is spi only
		cap_1bit 		= 1 << 2,		//! Host 1 bit transfer
		cap_4bit 		= 1 << 3,		//! Host 4 bit transfer
		cap_8bit 		= 1 << 4,		//! Host 8 bit transfer
		cap_hs   		= 1 << 5,		//! Capable run at hi speed
		cap_1_8V 		= 1 << 6,		//! 1v8 capable
		cap_uhs_sdr12   = 1 << 7,		//! UHS sdr 12
		cap_uhs_sdr25   = 1 << 8,		//! UHS sdr 25
		cap_uhs_sdr50   = 1 << 9,		//! UHS sdr 50
		cap_uhs_sdr104  = 1 << 10		//! UHS sdr 50
	};
	enum bus_width
	{
		bus_width_1b,
		bus_width_4b,
		bus_width_8b
	};
	enum ios_cmd
	{
		ios_pwr_off,
		ios_pwr_on,
		ios_set_bus_with,
		ios_set_speed
	};
	static const unsigned C_power_off = 0;
public:
	//Constructor
	mmc_host( immc_det_pin &detect_pin );
	//Destructor
	virtual ~mmc_host();
	//Get current allocated card
	mmc_card* get_card( int timeout = isix::ISIX_TIME_INFINITE);
	//Get card detect switch
	bool get_cd() const
	{
		return m_det_pin.get();
	}
	//Execute command and response check 
	int execute_command_resp_check(  mmc_command &req, unsigned timeout )
	{
	      int res; 
	      if( (res=execute_command(req,timeout)) ) return res;
	      res = req.get_err();
	      return res;
	}
	//Execute MMC command
	virtual int execute_command( mmc_command &req, unsigned timeout ) = 0;
	//Execute MMC data transfer
	virtual int send_data( const void *buf, size_t len, unsigned timeout ) = 0;
	//Execute MMC data transfer
	virtual int receive_data( void *buf, size_t len, unsigned timeout ) = 0;
	//Execute IO config
	virtual int set_ios( ios_cmd cmd, int param ) = 0;
	//Get capabilities
	virtual unsigned get_capabilities() const = 0;
	//Is SPI host
	bool is_spi() const { return get_capabilities() & cap_spi; }
private:
	//Raw insertion handler
	void det_card_insertion_callback();
	//Detect class insertion hander executed in interrupt content
	static void det_card_insertion_raw_callback( void* instance )
	{
		static_cast<mmc_host*>(instance)->det_card_insertion_callback();
	}
	//Reinitialize the card
	int reinitialize_card();
private:
	//Detect and allocate card
	mmc_card* mmc_allocate_card();
private:
	//MMC card
	mmc_card *m_card;
	//Card insertion removal PIN
	immc_det_pin& m_det_pin;
	//Virtual timer handler
	isix::vtimer_struct* m_det_timer;
	volatile bool m_card_init_req;
	volatile bool m_p_card_inserted;
	//Wait for card semaphore
	isix::semaphore m_card_sem;
};
/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/
#endif /* MMC_HOST_HPP_ */
/*----------------------------------------------------------*/
