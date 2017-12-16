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
#include <foundation/algo/noncopyable.hpp>
#include <cstddef>
#include <stdint.h>
#include "mmc/mmc_defs.hpp"

namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
class mmc_host;

/*----------------------------------------------------------*/
/* MMC card component */
class mmc_card :  public fnd::noncopyable
{
private:
		static const size_t C_sector_size = 512;
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
	/* Get card capacity */
	uint32_t get_sectors_count() const
	{
		return m_nr_sectors;
	}
	/* Get card CID */
	int get_cid( cid &c ) const;
	/* Get sector size */
	size_t get_sector_size() const
	{
		return C_sector_size;
	}
	/* Get erase size */
	int get_erase_size(uint32_t &sectors) const;
private:
	/* Write multiple block */
	inline int write_multi_blocks( const void* buf, unsigned long laddr,  std::size_t count );
	/* Read multiple block */
	inline int read_multi_blocks( void* buf, unsigned long laddr, std::size_t count );
	/* Write single block */
	inline int write_single_block( const void* buf, unsigned long laddr );
	/* Read multiple block */
	inline int read_single_block( void* buf, unsigned long laddr );
	//Update card parameters
	uint32_t read_csd_card_info();
    //Enable wide bus
    int sd_enable_wide_bus( int width );
    //Read extended card info
    int read_scr_card_info();
private:
	mmc_host& m_host;						//Host
	card_type m_type;						//Card type
	int m_error;							//Init error code
	uint16_t m_rca;							//RCA
	unsigned m_block_count_avail : 1;		//Block count avail
	unsigned m_bus_width : 2;				//Bus width
	uint32_t m_nr_sectors;					//Number of sectors on card
};

/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/

#endif /* MMC_CARD_HPP_ */
