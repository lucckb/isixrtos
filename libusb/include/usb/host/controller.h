/*
 * controller.h
 *
 *  Created on: 3 lut 2014
 *      Author: lucck
 */
  
#ifndef ISIX_LIBUSB_CONTROLLER_H_
#define ISIX_LIBUSB_CONTROLLER_H_
  
#include <stdint.h>
  
//!Fwd decl
struct usbh_driver;
  
#ifdef __cplusplus
extern "C" {
#endif

  
//! Initialize the USB host controller
int usbh_controller_init( int controller_id, int os_priority );

//! Register USBHost driver
int usbh_controller_attach_driver( const struct usbh_driver *drv );

//!Detach driver from host
int usbh_controller_detach_driver( const struct usbh_driver *drv );

//! Deinitialize controller driver
int usbh_controller_deinit( void );

//! Get last library error
int usbh_controller_get_errno( void );

//! Get Vendor ID
uint16_t usbh_get_id_vendor( void );

//! Get Product ID
uint16_t usbh_get_id_product( void );

  
#ifdef __cplusplus
}
#endif
 

#endif /* CONTROLLER_H_ */
