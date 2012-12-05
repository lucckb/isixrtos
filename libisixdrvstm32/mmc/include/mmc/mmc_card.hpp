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
	/* Constructor */
	explicit mmc_card( mmc_host &host )
	  : m_host(host) , m_type(type_none) {}
	int detect( );
public:
	/** Write the block */
	int write( const void* buf, unsigned long sector,  std::size_t count );
	/** Read the block */
	int read ( void* buf, unsigned long sector,  std::size_t count );
private:
	mmc_host& m_host;
	card_type m_type;
};

/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/

#endif /* MMC_CARD_HPP_ */
