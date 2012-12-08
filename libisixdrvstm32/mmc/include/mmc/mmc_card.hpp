/*
 * mmc_card.hpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */


/*----------------------------------------------------------*/
#ifndef MMC_CARD_HPP_
#define MMC_CARD_HPP_


/*----------------------------------------------------------*/
#include "noncopyable.hpp"
#include <cstddef>
#include <stdint.h>

namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
class mmc_host;

/*----------------------------------------------------------*/
/* MMC card component */
class mmc_card :  public fnd::noncopyable
{
public:
	enum card_type
	{
		type_none,		//Card not initialized
		type_mmc,		//MMC card
		type_sd_v1,		//SD V1
		type_sd_v2,		//SD V2
		type_sdhc		//SDHC
	};
	static int detect( mmc_host &host, mmc_card* &old_card );
private:
	//initialize card in SD mode
	int sd_mode_initialize();
	/* Constructor */
	explicit mmc_card( mmc_host &host, card_type type );
	//Hardware probe the card and init it
	static int probe( mmc_host &host, card_type &type );
	//Get error code
	int get_error() const
	{
		return m_error;
	}
public:
	/** Write the block */
	int write( const void* buf, unsigned long sector,  std::size_t count );
	/** Read the block */
	int read( void* buf, unsigned long sector, std::size_t count );
private:
	mmc_host& m_host;
	card_type m_type;
	int m_error;
	uint16_t m_rca;
	unsigned m_block_count_avail : 1;
	unsigned m_bus_width : 2;
};

/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/

#endif /* MMC_CARD_HPP_ */
