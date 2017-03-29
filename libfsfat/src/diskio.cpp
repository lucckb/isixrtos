
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2012        */

/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */


#include "diskio.h"		/* FatFs lower layer API */
#include <config/conf.h>
#include "mmc/mmc_slot.hpp"
#include "mmc/mmc_card.hpp"

#define MAX_DISK_NUMB 2
static drv::mmc::mmc_slot* g_slots[MAX_DISK_NUMB];
static drv::mmc::mmc_card* g_cards[MAX_DISK_NUMB];

DRESULT disk_add( BYTE disk_id, drv::mmc::mmc_slot *slot )
{
	if( disk_id >= MAX_DISK_NUMB ) return RES_PARERR;
	g_slots[disk_id] = slot;
	return RES_OK;
}

extern "C"
{
	DWORD __attribute__ ((weak)) get_fattime(void)
	{
		return 0;
	}
}

/* Initialize a Drive                                                    */


DSTATUS disk_initialize (
		BYTE drv				/* Physical drive nmuber (0..) */
)
{
	using namespace drv::mmc;
	if( drv >= MAX_DISK_NUMB ) return RES_PARERR;
	if( !g_slots[drv] ) return STA_NOINIT;
	if( g_slots[drv]->check( mmc_slot::C_no_block ) & mmc_slot::card_removed )
	{
		return STA_NODISK;
	}
	if( g_slots[drv]->get_card( g_cards[drv] ) != MMC_OK )
	{
		g_cards[drv] = NULL;
		return STA_NOINIT;
	}
	return 0;
}

/* Get Disk Status                                                       */


DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	using namespace drv::mmc;
	if( drv >= MAX_DISK_NUMB ) return RES_PARERR;
	if( !g_slots[drv] ) return STA_NOINIT;

	if( g_slots[drv]->check( mmc_slot::C_no_block ) & mmc_slot::card_removed )
		return STA_NODISK;
	return 0;
}


/* Read Sector(s)                                                        */


DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
	using namespace drv::mmc;
	if( drv >= MAX_DISK_NUMB ) return RES_PARERR;
	if( !g_slots[drv]  ) return RES_ERROR;
	if( !g_cards[drv] )  return RES_NOTRDY;
	switch( g_cards[drv]->read( buff, sector, count ) )
	{
	case MMC_CARD_NOT_PRESENT:
		return RES_NOTRDY;
	case MMC_OK:
		return RES_OK;
	default:
		return RES_ERROR;
	}
}


/* Write Sector(s)                                                       */


#if _USE_WRITE
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
	using namespace drv::mmc;
	if( drv >= MAX_DISK_NUMB ) return RES_PARERR;
	if( !g_slots[drv]  ) return RES_ERROR;
	if( !g_cards[drv] )  return RES_NOTRDY;
	switch( g_cards[drv]->write( buff, sector, count ) )
	{
	case MMC_CARD_NOT_PRESENT:
		return RES_NOTRDY;
	case MMC_OK:
		return RES_OK;
	default:
		return RES_ERROR;
	}
}
#endif



/* Miscellaneous Functions                                               */


#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	(void)drv; (void)buff;
	switch( ctrl )
	{
	case CTRL_SYNC:
		return RES_OK;
	default:
		return RES_ERROR;
	}
}
#endif
