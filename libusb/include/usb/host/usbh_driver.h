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
#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------ */ 
struct usbhost_device;
/* ------------------------------------------------------------------ */ 
enum usbh_driver_ret {
	usbh_driver_ret_configured,		//Device configured
	usbh_driver_ret_not_found,		//Device not found
};
/* ------------------------------------------------------------------ */ 
//!Private usbhdriver struct
struct usbh_driver {
	int (*attached)( const struct usbhost_device* hdev, void** data );
	int (*process)(void* data);		//! Process data
};
/* ------------------------------------------------------------------ */ 
#ifdef __cplusplus
}
#endif
/* ------------------------------------------------------------------ */
