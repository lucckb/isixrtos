/*
 * controller.h
 *
 *  Created on: 3 lut 2014
 *      Author: lucck
 */

#ifndef ISIX_LIBUSB_CONTROLLER_H_
#define ISIX_LIBUSB_CONTROLLER_H_

#include <stdint.h>
#include <isix/prv/list.h>

struct usbh_driver {
	
	uint8_t dev_class;
	uint8_t dev_subclass;
	int (*on_connect)(void*);
	int (*deinit)(void*);
	list_t inode;
};

//! Initialize the USB host controller
void usbh_controller_init( void );

//! Register USBHost driver
void usbh_controller_register_driver( const usbh_driver *drv );

//! Unregister the driver

//Deinitialize controller driver
void usbh_controller_deinit( void );



#endif /* CONTROLLER_H_ */
