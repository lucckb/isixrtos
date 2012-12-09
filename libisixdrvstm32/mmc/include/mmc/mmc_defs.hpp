/*
 * mmc_defs.hpp
 *
 *  Created on: 09-12-2012
 *      Author: lucck
 */

#ifndef MMC_DEFS_HPP_
#define MMC_DEFS_HPP_

namespace drv {
namespace mmc {


/*----------------------------------------------------------*/
struct cid
{
    unsigned int        manfid;
    char            	prod_name[8];
    unsigned int        serial;
    unsigned short      oemid;
    unsigned short      year;
    unsigned char       hwrev;
    unsigned char       fwrev;
    unsigned char       month;
};

/*----------------------------------------------------------*/
}}
/*----------------------------------------------------------*/
#endif /* MMC_DEFS_HPP_ */
