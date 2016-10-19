/*
 * =====================================================================================
 *
 *       Filename:  phy.h
 *
 *    Description:  PHY interface header
 *
 *        Version:  1.0
 *        Created:  31.08.2016 17:38:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>



enum _ethernetif_phy_link {
	 phy_link_auto = 1<<12,
	 phy_link_100m = 1<<13,
	 phy_link_fduplex = 1<<8
};

enum _ethernetif_phy_error {
	phyerr_ok = 0,
	phyerr_invid = -256,
	phyerr_reset = -257,
	phyerr_autoneg = -258,
	phyerr_intr = -259,
};


enum _ethernetif_phy_status {
	phy_stat_none = 0,
	phy_stat_extcap = 1<<0,
	phy_stat_jabber = 1<<1,
	phy_stat_linkup = 1<<2,
	phy_stat_can_autoneg = 1<<3,
	phy_stat_remote_fault = 1<<4,
	phy_stat_autoneg_compl = 1<<5,
	phy_stat_no_preamble = 1<<6,
	phy_stat_10m_half = 1<<11,
	phy_stat_10m_full = 1<<12,
	phy_stat_100m_half = 1<<13,
	phy_stat_100m_full = 1<<14,
	phy_stat_100m_t4 = 1<<15
};

struct phy_device {


	//! Configure ethernet interface
	//  Need check if comm is ok and reset it
	int (*probe)( uint8_t addr, int link_type );

	// Reset EMAC phy
	int (*reset)( uint8_t addr );

	//! Configure and enable disable interrupts
	int (*config_intr)( uint8_t addr, bool en );

	//! Configure auto negotiation
	int(*config_speed)( uint8_t addr, uint16_t flags );

	//! Get the current link status
	int (*read_status)( uint8_t addr );

	};



