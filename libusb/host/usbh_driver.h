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

#ifndef  _INTERNAL_USBH_DRIVER_STRUCT_H
#define  _INTERNAL_USBH_DRIVER_STRUCT_H
/* ------------------------------------------------------------------ */ 
#include <stdint.h>
#include <stdbool.h>
/* ------------------------------------------------------------------ */ 
//!Private usbhdriver struct
struct usbh_driver {
	uint16_t dev_class;				//! Device class
	uint16_t dev_subclass;			//! Device subclass
	uint16_t dev_vid;				//! Vendor ID
	uint16_t dev_pid;				//! Product ID
	int (*process)(void*);			//! Process data
};
/* ------------------------------------------------------------------ */ 
#endif   /* ----- #ifndef usbh_driver_INC  ----- */
/* ------------------------------------------------------------------ */ 
