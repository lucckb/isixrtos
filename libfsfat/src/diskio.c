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

/*-----------------------------------------------------------------------*/
#define ISIX_FATFS_USE_SPI_MODE 1
#define ISIX_FATFS_USE_SDIO_MODE 2

/*-----------------------------------------------------------------------*/
#ifndef ISIX_FATFS_MODE
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
#define ISIX_FATFS_MODE ISIX_FATFS_USE_SDIO_MODE
#else
#define ISIX_FATFS_MODE ISIX_FATFS_USE_SPI_MODE
#endif
#endif

/*-----------------------------------------------------------------------*/
#if (ISIX_FATFS_MODE==ISIX_FATFS_USE_SDIO_MODE)
#include "sdio_sdcard_driver.h"
#elif (ISIX_FATFS_MODE==ISIX_FATFS_USE_SPI_MODE)
#include "spi_sdcard_driver.h"
#else
#error "Unknown SPI driver mode"
#endif
/*-----------------------------------------------------------------------*/
#if (ISIX_FATFS_MODE==ISIX_FATFS_USE_SDIO_MODE)
#define _sdcard_init isix_sdio_card_driver_init
#define _scard_reinitialize isix_sdio_card_driver_reinitialize
#define _sdcard_is_card_in_slot isix_sdio_card_driver_is_card_in_slot
#define _sdcard_card_driver_status isix_sdio_card_driver_status
#define _sdcard_read isix_sdio_card_driver_read
#define _sdcard_write isix_sdio_card_driver_write
#elif (ISIX_FATFS_MODE==ISIX_FATFS_USE_SPI_MODE)
#define _sdcard_init isix_spisd_card_driver_init
#define _scard_reinitialize isix_spisd_card_driver_reinitialize
#define _sdcard_is_card_in_slot isix_spisd_card_driver_is_card_in_slot
#define _sdcard_card_driver_status isix_spisd_card_driver_status
#define _sdcard_read isix_spisd_card_driver_read
#define _sdcard_write isix_spisd_card_driver_write
#endif
/*-----------------------------------------------------------------------*/
DWORD get_fattime(void)
{
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */

/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
		BYTE drv				/* Physical drive nmuber (0..) */
)
{
	(void)drv;
	switch ( _sdcard_init() )
	{
	case SD_LIB_ALREADY_INITIALIZED:
		if( _scard_reinitialize() )
			return RES_ERROR;
		else
			return RES_OK;
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
	if( !_sdcard_is_card_in_slot() )
		return STA_NODISK;
	switch( _sdcard_card_driver_status() )
	{
	case SDCARD_DRVSTAT_NOINIT:
		return STA_NOINIT;
	case SDCARD_DRVSTAT_OK:
	case SDCARD_DRVSTAT_BUSY:
		return 0;
	default:
		return 0;
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
	if( _sdcard_read(buff, sector, count) )
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
	if( _sdcard_write( buff, sector, count) )
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
