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
int stm32_usbdev_serial_init( size_t rx_fifo_len, size_t tx_fifo_len  );

/* ------------------------------------------------------------------ */
/* USB serial putchar */
int stm32_usbdev_serial_putc( int c );

/* ------------------------------------------------------------------ */
/* USB serial get char */
int stm32_usbdev_serial_getc( );

/* ------------------------------------------------------------------ */


/* ------------------------------------------------------------------ */
#ifdef __cplusplus
}
}
}
#endif

/* ------------------------------------------------------------------ */
#endif /* USBSERIAL_H_ */
