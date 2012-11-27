/* ------------------------------------------------------------------ */
/*
 * sdio_card_driver.h
 *
 *  Created on: 22-11-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef SDIO_CARD_DRIVER_H_
#define SDIO_CARD_DRIVER_H_
/* ------------------------------------------------------------------ */
#include "sdcard_driver_common.h"
/* ------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
namespace stm32 {
namespace drv {
#endif

/* ------------------------------------------------------------------ */
//Initialize the SDIO card driver
int isix_sdio_card_driver_init(void);

/* ------------------------------------------------------------------ */
//Card driver status
sdcard_drvstat isix_sdio_card_driver_status(void);

/* ------------------------------------------------------------------ */
//SDIO card driver read
int isix_sdio_card_driver_read( void *buf, unsigned long LBA, size_t count );

/* ------------------------------------------------------------------ */
//SDIO card driver write
int isix_sdio_card_driver_write( const void *buf, unsigned long LBA, size_t count );

/* ------------------------------------------------------------------ */
/* Destroy card driver and disable interrupt */
void isix_sdio_card_driver_destroy(void);

/* ------------------------------------------------------------------ */
//Get SD card info
int isix_sdio_card_driver_get_info( void *cardinfo, scard_info_field req );

/* ------------------------------------------------------------------ */
//SD CARD initialize again
int isix_sdio_card_driver_reinitialize( void );

/* ------------------------------------------------------------------ */
//IS card present
bool isix_sdio_card_driver_is_card_in_slot( void );

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
}}}
#endif

/* ------------------------------------------------------------------ */
#endif /* SDIO_CARD_DRIVER_H_ */
