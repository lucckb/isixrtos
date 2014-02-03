/*
 * controller.h
 *
 *  Created on: 3 lut 2014
 *      Author: lucck
 */

#ifndef ISIX_LIBUSB_CONTROLLER_H_
#define ISIX_LIBUSB_CONTROLLER_H_

#include <stdint.h>


struct usbh_driver {
	uint8_t dev_class;
	uint8_t dev_subclass;
	int (*init)(void*);
	int (*deinit)(void*);
};

void usbh_controller_init( void );

void usbh_controller_register_driver( const usbh_driver *drv );

void usbh_controller_deinit( void );



#endif /* CONTROLLER_H_ */
