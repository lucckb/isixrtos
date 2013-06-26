/*
 * usbhost.h
 *
 *  Created on: 20-06-2013
 *      Author: lucck
 */

#ifndef LIBISIX_DRV_USBHIDKBD_HPP_
#define LIBISIX_DRV_USBHIDKBD_HPP_
/* ------------------------------------------------------------------ */
#include <stddef.h>
#include <isix/dev/input.hpp>

/* ------------------------------------------------------------------ */
namespace stm32 {
namespace dev {
/* ------------------------------------------------------------------ */
/* Initialize USB bus */
int usb_bus_initialize( );

/* ------------------------------------------------------------------ */
/* Register usb device class */
int usb_register_device( isix::dev::device &device );

/* ------------------------------------------------------------------ */
class hid_keyboard	: public isix::dev::input_class
{

};
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* USBHOST_H_ */
