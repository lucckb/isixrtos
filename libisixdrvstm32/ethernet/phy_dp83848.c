/*
 * =====================================================================================
 *
 *       Filename:  phy_dp83848.c
 *
 *    Description:  PHY83848 dev
 *
 *        Version:  1.0
 *        Created:  31.08.2016 18:24:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include "ethernetif_prv.h"
#include <stm32_eth.h>
#include <eth/phy.h>


//DP83848 specific bits
//PHY registers and bits
enum { MICR_INTEN = (1<<1),  MICR_INTOE = (1<<0 ) };

enum { MICSR_ANC_INT_EN=(1<<2), MICSR_DUP_INT_EN=(1<<3), MICSR_SPD_INT_EN=(1<<4),
		MICSR_LINK_INT_EN=(1<<5), MICSR_LINK_INT=(1<<13) };

enum { PHYBCR=0x00, PHYBMSR=0x01, PHYIDR1=0x02,
	   PHYIDR2=0x03, PHYMICR=0x11,PHYMICSR=0x12, PHYCR=0x19
};



//! Configure ethernet interface
//  Need check if comm is ok and reset it
static int dp83848_probe( uint8_t addr, int link_type )
{
	(void)link_type;
	/* konfiguracja diod świecących na ZL3ETH
	   zielona - link status: on = good link, off = no link, blink = activity
	   pomarańczowa - speed: on = 100 Mb/s, off = 10 Mb/s
	   */

	enum { LED_CNFG0 = 0x0020 };
	enum { LED_CNFG1 = 0x0040 };
	enum { DP83848_ID = 0x080017 };
	uint32_t phy_idcode = (((uint32_t)_ethernetif_read_phy_register_( addr, PHYIDR1)<<16) |
			_ethernetif_read_phy_register_( addr, PHYIDR2)) >> 10;
	if( phy_idcode != DP83848_ID ) {
		return phyerr_invid;
	}
	uint16_t phyreg = _ethernetif_read_phy_register_( addr, PHYCR);
	phyreg &= ~(LED_CNFG0 | LED_CNFG1);
	_ethernetif_write_phy_register_( addr, PHYCR, phyreg);
	return phyerr_ok;
}


// Reset EMAC phy
static int dp83848_reset( uint8_t addr )
{
	if( _ethernetif_write_phy_register_(addr, PHYBCR, PHY_Reset) ) {
		return phyerr_reset;
	} else {
		return phyerr_ok;
	}
}


//! Get the current link status
static int dp83848_read_status( uint8_t addr )
{
	const uint16_t sr = _ethernetif_read_phy_register_( addr, PHYMICSR );
	if( sr & MICSR_LINK_INT ) {
		return _ethernetif_read_phy_register_( addr,  PHYBMSR  );
	} else {
		return phy_stat_none;
	}
}


//! Configure auto negotiation
static int dp83848_config_speed( uint8_t addr, uint16_t flags )
{
	if(_ethernetif_write_phy_register_( addr, PHY_BCR, flags ) ) {
		return phyerr_autoneg;
	} else {
		return phyerr_ok;
	}
}


//! Configure and enable disable interrupts
static int dp83848_config_intr( uint8_t addr, bool en )
{
	/* Configure PHY for link status interrupt gen */
	if( _ethernetif_write_phy_register_( addr, PHYMICR,  en?(MICR_INTEN | MICR_INTOE):0 ) )
	{
		return phyerr_intr;
	}
	if( _ethernetif_write_phy_register_( addr, PHYMICSR,  en?MICSR_LINK_INT_EN:0) )
	{
		return phyerr_intr;
	}
	return phyerr_ok;
}


const struct phy_device _ethernetif_phy_dp83848_drv_ = {
	.probe = dp83848_probe,
	.reset = dp83848_reset,
	.read_status = dp83848_read_status,
	.config_speed = dp83848_config_speed,
	.config_intr = dp83848_config_intr
};




