/*
 * =====================================================================================
 *
 *       Filename:  hid_joystick.c
 *
 *    Description:  HID joystick driver implementation
 *
 *        Version:  1.0
 *        Created:  22.02.2014 13:21:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <usb/drivers/hostdev/hid_joystick.h>
#include <usb/host/usbh_driver.h>
#include <foundation/dbglog.h>
#include <usb/core/usbh_config_descriptors.h>
#include <usb/core/usb_def.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_hid_req.h>
#include <usb/core/usbh_hid_core.h>
#include <usb/host/usb_device_struct.h>
#include <usb/core/usbh_std_req.h>
#include <isix.h>
#include <stdlib.h>
#include <string.h>
/* ------------------------------------------------------------------ */ 
//!Joystick operation structure
static const usbh_hid_joystick_ops_t* g_joy_ops;
/* ------------------------------------------------------------------ */ 
//! Driver auto detect and attach
static int hid_joystick_attached( const usbhost_device_t* hdev, void** data )
{
	dbprintf("Joystick attached");
	(void)hdev;
	(void)data;
	return usbh_driver_ret_not_found;
}
/* ------------------------------------------------------------------ */ 
//Enumerate descriptor
static void hid_joystick_enum_desc( void *data, enum usbh_driver_desc_type desc, const char *str )
{
	if( g_joy_ops && g_joy_ops->enum_desc ) {
		g_joy_ops->enum_desc( data, desc, str );
	}
}
/* ------------------------------------------------------------------ */ 
//! Driver keyboard process
static int hid_joystick_process( void* data )
{
	(void)data;
	return -1;
}
/* ------------------------------------------------------------------ */ 
//! Driver OPS structure
static const usbh_driver_t drv_ops = {
	hid_joystick_attached,
	hid_joystick_process,
	hid_joystick_enum_desc
};
/* ------------------------------------------------------------------ */ 
//Joystick Driver init
const struct usbh_driver*
	usbh_hid_joystick_init( const usbh_hid_joystick_ops_t* joy_ops )
{
	g_joy_ops = joy_ops;
	return &drv_ops;
}
/* ------------------------------------------------------------------ */ 
