/*
 * =====================================================================================
 *
 *       Filename:  Internal usb driver structure
 *
 *    Description:  Internal USB driver structure def
 *
 *        Version:  1.0
 *        Created:  05.02.2014 22:19:50
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
/* ------------------------------------------------------------------ */ 
#include <stdint.h>
#include <stdbool.h>
/* ------------------------------------------------------------------ */ 
struct usb_descriptor_data;
/* ------------------------------------------------------------------ */ 
#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------ */ 
enum usbh_driver_ret {
	usbh_driver_ret_configured,		//Device configured
	usbh_driver_ret_not_found		//Device not found
};
/* ------------------------------------------------------------------ */ 
//!Private usbhdriver struct
struct usbh_driver {
	int (*attached)(const struct usb_descriptor_data* desc );
	int (*process)(void);			//! Process data
};
/* ------------------------------------------------------------------ */ 
#ifdef __cplusplus
}
#endif
/* ------------------------------------------------------------------ */
