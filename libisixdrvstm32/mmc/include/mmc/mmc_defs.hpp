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
struct scr
{
    unsigned char spec;
    unsigned char bus_width_1b : 1;
    unsigned char bus_width_4b : 1;
    unsigned char is_set_block_count: 1;
    unsigned char is_speed_class: 1;
};
/*----------------------------------------------------------*/
}}
/*----------------------------------------------------------*/
#endif /* MMC_DEFS_HPP_ */
