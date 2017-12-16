/*
 * =====================================================================================
 *
 *       Filename:  phy_ksz8081.c
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
#include <foundation/sys/dbglog.h>


enum regs {
	PHYBCR  =  0x00,
	PHYBSR  =  0x01,
	PHYIDR1 = 0x02,
	PHYIDR2 = 0x03,
	PHYICSR  = 0x1B
};

enum ics_bits {
	ICS_LINK_UP = 1<<0,
	ICS_LINK_FAULT = 1<<1,
	ICS_LINK_DOWN = 1<<2,
	ICS_LINK_UP_INTEN = 1<<8,
	ICS_LINK_DOWN_INTEN = 1<<10
};

//! Configure ethernet interfacea
//  Need check if comm is ok and reset it
static int ksz8081_probe( uint8_t addr, int link_type )
{
	(void)link_type;
	/* konfiguracja diod świecących na ZL3ETH
	   zielona - link status: on = good link, off = no link, blink = activity
	   pomarańczowa - speed: on = 100 Mb/s, off = 10 Mb/s
	   */
	enum {
		ksz8081_ID = 0x00221560,
	};

	uint32_t phy_idcode = (((uint32_t)_ethernetif_read_phy_register_( addr, PHYIDR1)<<16) |
			_ethernetif_read_phy_register_( addr, PHYIDR2));
	if( (phy_idcode&~1U) != ksz8081_ID  ) {
		dbg_err("Not a KSZ8081 PHYs: %08x", phy_idcode );
		return phyerr_invid;
	}
#if 0
	uint16_t phyreg = _ethernetif_read_phy_register_( addr, PHYCR);
	phyreg &= ~(LED_CNFG0 | LED_CNFG1);
	_ethernetif_write_phy_register_( addr, PHYCR, phyreg);
#endif
	return phyerr_ok;
}


// Reset EMAC phy
static int ksz8081_reset( uint8_t addr )
{
	if( _ethernetif_write_phy_register_(addr, PHYBCR, PHY_Reset) ) {
		return phyerr_reset;
	} else {
		return phyerr_ok;
	}
}


//! Get the current link status
static int ksz8081_read_status( uint8_t addr )
{
	const uint16_t sr = _ethernetif_read_phy_register_( addr, PHYICSR );
	if( sr & (ICS_LINK_UP|ICS_LINK_DOWN|ICS_LINK_FAULT) ) {
		return _ethernetif_read_phy_register_( addr,  PHYBSR  );
	} else {
		return phy_stat_none;
	}
}


//! Configure auto negotiation
static int ksz8081_config_speed( uint8_t addr, uint16_t flags )
{
	if(_ethernetif_write_phy_register_( addr, PHY_BCR, flags ) ) {
		return phyerr_autoneg;
	} else {
		return phyerr_ok;
	}
}


//! Configure and enable disable interrupts
static int ksz8081_config_intr( uint8_t addr, bool en )
{
	/* Configure PHY for link status interrupt gen */
	if( _ethernetif_write_phy_register_( addr, PHYICSR,
				en?(ICS_LINK_UP_INTEN|ICS_LINK_DOWN_INTEN):0 ) )
	{
		return phyerr_intr;
	}
	return phyerr_ok;
}


const struct phy_device _ethernetif_phy_ksz8081_drv_ = {
	.probe = ksz8081_probe,
	.reset = ksz8081_reset,
	.read_status = ksz8081_read_status,
	.config_speed = ksz8081_config_speed,
	.config_intr = ksz8081_config_intr
};




