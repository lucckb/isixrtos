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
int stm32_usbdev_serial_open( void );

/* ------------------------------------------------------------------ */
/* Write data to the virtual serial com port
 * @param[in] buf Pointer to data buffer
 * @param[in] buf_len Buffer length
 * @param[in] timeout Timeout or ISIX_TIME_INFINITE
 * @return Number of bytes written or negative error code if fail
 */
int stm32_usbdev_serial_write( const void *buf, size_t buf_len, unsigned timeout );

/* ------------------------------------------------------------------ */
enum stm32_usbdev_serial_timeout_mode			//STM32 block timeout mode
{
	USBDEV_SERIAL_NONBLOCK			  =  0,      //Nonblock mode
	USBDEV_SERIAL_BLOCK_TO_LEN 		  = -1,		//!Block to request length
	USBDEV_SERIAL_BLOCK_TO_DATA_AVAIL = -2,		//!Block to first data avail
};
/* ------------------------------------------------------------------ */
/* read data from the virtual serial com port
 * @param[out] buf Pointer to data buffer
 * @param[in] buf_len Buffer length
 * @param[in] timeout Positive timeout or @see stm32_usbdev_serial_timeout_mode
 * @return Number of bytes read or negative error code if fail
 */
int stm32_usbdev_serial_read( void *buf, size_t buf_len, int tout_mode);

/* ------------------------------------------------------------------ */

#ifdef __cplusplus
}}}
#endif

/* ------------------------------------------------------------------ */
#endif /* USBSERIAL_H_ */
