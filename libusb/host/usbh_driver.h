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
#include <prv/list.h>
/* ------------------------------------------------------------------ */ 
//!Private usbhdriver struct
struct usbh_driver {
	
	uint8_t dev_class;				//! Device class
	uint8_t dev_subclass;			//! Device subclass
	int (*connect)(void*);			//! Connect and descriptor readed
	int (*disconnect)(void*);		//! Disconnect from host
	list_t inode;
};

/* ------------------------------------------------------------------ */ 
#endif   /* ----- #ifndef usbh_driver_INC  ----- */
/* ------------------------------------------------------------------ */ 
