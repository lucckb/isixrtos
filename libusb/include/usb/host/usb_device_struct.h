/*
 * =====================================================================================
 *
 *       Filename:  usb_device_struct.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06.02.2014 20:42:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

/* ------------------------------------------------------------------ */ 

#ifndef  ISIX_LIBUSB_USB_DEVICE_STRUCT_H_
#define  ISIX_LIBUSB_USB_DEVICE_STRUCT_H_
/* ------------------------------------------------------------------ */ 
#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */ 
//! USB descriptor data
struct usb_descriptor_data {
	void* mem;
	uint16_t size;
};
/* ------------------------------------------------------------------ */ 
//! USB host device description
struct usbhost_device {
	//Part of the connected device data
	usb_device_descriptor_t dev_desc;	//Device descriptor
	usb_speed_t speed;					//Device speed
	uint8_t dev_addr;					//Device address
	struct usb_descriptor_data cfg_desc;//Config descriptor
};

/* ------------------------------------------------------------------ */ 
#ifdef __cplusplus
}
#endif
/* ------------------------------------------------------------------ */
#endif   /* ----- #ifndef usb_device_struct_INC  ----- */
