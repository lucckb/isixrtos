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

  
#pragma once
  
#ifdef __cplusplus
extern "C" {
#endif
  
//! USB host device description
struct usbhost_device {
	//Part of the connected device data
	usb_device_descriptor_t dev_desc;	//Device descriptor
	usb_speed_t speed;					//Device speed
	uint8_t dev_addr;					//Device address
	void* cdesc;						//Config descriptor pointer
	uint16_t cdsize;					//Config descriptor size
	void *data;							//Configuration data
};

typedef struct usbhost_device usbhost_device_t;

  
#ifdef __cplusplus
}
#endif
 
