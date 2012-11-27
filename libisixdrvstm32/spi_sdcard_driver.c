/*
 * spi_sdcard_driver.c
 *
 *  Created on: 27-11-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include "spi_sdcard_driver.h"
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif

/* ------------------------------------------------------------------ */

//Initialize the SDIO card driver
int isix_spisd_card_driver_init(void)
{

}

/* ------------------------------------------------------------------ */
//Card driver status
sdcard_drvstat isix_spisd_card_driver_status(void)
{

}

/* ------------------------------------------------------------------ */
//SDIO card driver read
int isix_spisd_card_driver_read( void *buf, unsigned long LBA, size_t count )
{

}

/* ------------------------------------------------------------------ */
//SDIO card driver write
int isix_spisd_card_driver_write( const void *buf, unsigned long LBA, size_t count )
{

}

/* ------------------------------------------------------------------ */
/* Destroy card driver and disable interrupt */
void isix_spisd_card_driver_destroy(void)
{

}

/* ------------------------------------------------------------------ */
//Get SD card info
int isix_spisd_card_driver_get_info( void *cardinfo, scard_info_field req )
{

}

/* ------------------------------------------------------------------ */
//SD CARD initialize again
int isix_spisd_card_driver_reinitialize( void )
{

}

/* ------------------------------------------------------------------ */
//IS card present
bool isix_spisd_card_driver_is_card_in_slot( void )
{

}

/* ------------------------------------------------------------------ */
