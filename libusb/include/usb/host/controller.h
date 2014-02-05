/*
 * controller.h
 *
 *  Created on: 3 lut 2014
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */ 
#ifndef ISIX_LIBUSB_CONTROLLER_H_
#define ISIX_LIBUSB_CONTROLLER_H_
/* ------------------------------------------------------------------ */ 
#include <stdint.h>
/* ------------------------------------------------------------------ */ 
//!Fwd decl
struct usbh_driver;

/* ------------------------------------------------------------------ */ 

//! Initialize the USB host controller
int usbh_controller_init( int controller_id );

//! Register USBHost driver
int usbh_controller_attach_driver( const struct usbh_driver *drv );

//!Detach driver from host
int usbh_controller_detach_driver( const struct usbh_driver *drv );

//Deinitialize controller driver
int usbh_controller_deinit( void );

/* ------------------------------------------------------------------ */ 

#endif /* CONTROLLER_H_ */
