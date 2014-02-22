/*
 * =====================================================================================
 *
 *       Filename:  hid_joystick.h
 *
 *    Description:  HID joystick driver
 *
 *        Version:  1.0
 *        Created:  22.02.2014 10:51:47
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
#include <usb/host/usbh_driver_desc_type.h>

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------ */
//!Forward struct decl
struct usbh_hid_joy_context;
typedef struct usbh_hid_joy_context usbh_hid_joy_context_t;

//!Joystick info
struct usbh_hid_joystick_info {
	uint8_t has_throttle : 1;			//! Joystick has throotle
	uint8_t has_hat : 1;				//! Joystic has hat
	uint8_t buttons_count;				//! Joystic button count
	short XY_min;						//! XY min value
	short XY_max;						//! XY max value
	short throttle_min;					//! Throttle minimum value
	short throttle_max;					//! Throttle maximum value
	short hat_min;						//! Hat minimum value
	short hat_max;						//! Hat maximum value
};
typedef struct usbh_hid_joystick_info usbh_hid_joystick_info_t;

//! Joystick keyboard hid event
struct usbh_joy_hid_event {
	unsigned buttons;		//! Button bitmaps
	short X;				//! X axis
	short Y;				//! Y axis
	short throttle;			//! Throttle
	short hat;				//! Hat switch
};
typedef struct usbh_joy_hid_event usbh_joy_hid_event_t;

//! Joystick structure operation
struct usbh_hid_joystick_ops {
	void ( *connected )( const usbh_hid_joy_context_t* id );			//New keyb event
	void ( *disconnected )( const usbh_hid_joy_context_t* id );		//Disconnect event
	void ( *report )( const usbh_hid_joy_context_t *id, const usbh_joy_hid_event_t* evt ); // Report event
	void (*enum_desc)( const usbh_hid_joy_context_t *id, enum usbh_driver_desc_type desc, const char *str );
	void (*enum_joyinfo)( const usbh_hid_joy_context_t* id, const usbh_hid_joystick_info_t* info );
};
typedef struct usbh_hid_joystick_ops usbh_hid_joystick_ops_t;
/* ------------------------------------------------------------------ */ 
//! Initialize the joystick HID driver
const struct usbh_driver*
	usbh_hid_joystick_init( const usbh_hid_joystick_ops_t* joy_ops );
/* ------------------------------------------------------------------ */ 
#ifdef __cplusplus
}
#endif
