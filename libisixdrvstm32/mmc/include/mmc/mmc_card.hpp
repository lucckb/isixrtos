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

namespace drv {
/*----------------------------------------------------------*/
class mmc_host;

/*----------------------------------------------------------*/
/* MMC card component */
class mmc_card :  public fnd::noncopyable
{
public:
	/* Constructor */
	mmc_card( mmc_host &host )
		: m_host(host) {}
	/** Write the block */
	int write( const void* buf, unsigned long sector,  std::size_t count );
	/** Read the block */
	int read ( void* buf, unsigned long sector,  std::size_t count );
	//** Initialize the card on request
	int initialize();
private:
	mmc_host& m_host;
};

/*----------------------------------------------------------*/
} /* namespace drv */

/*----------------------------------------------------------*/

#endif /* MMC_CARD_HPP_ */
