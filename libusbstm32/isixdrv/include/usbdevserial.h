/*
 * usbserial.h
 *
 *  Created on: 10-11-2012
 *      Author: lucck
 */

#ifndef LIBISIX_DRV_USBDEVSERIAL_H_
#define LIBISIX_DRV_USBDEVSERIAL_H_
/* ------------------------------------------------------------------ */
#include <stddef.h>

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
namespace stm32 {
namespace dev {
#endif

/* ------------------------------------------------------------------ */
/* Initialize the USB serial module */
int stm32_usbdev_serial_init( void  );

/* ------------------------------------------------------------------ */
/* USB serial putchar */
int stm32_usbdev_serial_putc( int c );

/* ------------------------------------------------------------------ */
/* USB serial get char */
int stm32_usbdev_serial_getc( );

/* ------------------------------------------------------------------ */
/* Write data to the stm32 device */
int stm32_usbdev_write( const void *buf, size_t buf_len );
/* ------------------------------------------------------------------ */
#ifdef __cplusplus
}
}
}
#endif

/* ------------------------------------------------------------------ */
#endif /* USBSERIAL_H_ */
