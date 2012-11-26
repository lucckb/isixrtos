/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2012        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef LIBISIX_FAT_USE_SDIO
#define LIBISIX_FAT_USE_SDIO
#endif

#ifdef LIBISIX_FAT_USE_SDIO
#include "sdio_sdcard_driver.h"
#endif

/*-----------------------------------------------------------------------*/
DWORD get_fattime(void)
{
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
		BYTE drv				/* Physical drive nmuber (0..) */
)
{
	(void)drv;
	switch ( isix_sdio_card_driver_init() )
	{
	case SD_LIB_ALREADY_INITIALIZED:
		if( isix_sdio_card_driver_reinitialize() )
			return RES_OK;
		else
			return RES_ERROR;
	case SD_OK:
			return RES_OK;
	default:
		return RES_ERROR;
	}
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	(void)drv;
	if( !isix_sdio_card_driver_is_card_in_slot() )
		return STA_NODISK;
	switch( isix_sdio_card_driver_status() )
	{
	case SDCARD_DRVSTAT_NOINIT:
		return STA_NOINIT;
	case SDCARD_DRVSTAT_OK:
		return 0;
	default:
		return STA_NOINIT;
	}
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
	(void)drv;
	if( isix_sdio_card_driver_read(buff, sector, count) )
	{
		return RES_ERROR;
	}
	else
		return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
	(void)drv;
	if( isix_sdio_card_driver_write( buff, sector, count) )
	{
		return RES_ERROR;
	}
	else
		return RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

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
