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
  
#include <stdint.h>
#include <stdbool.h>
#include <usb/host/usbh_driver_desc_type.h>

 
#ifdef __cplusplus
extern "C" {
#endif
 
//!Forward struct decl
struct usbh_hid_joy_context;
typedef struct usbh_hid_joy_context usbh_hid_joy_context_t;


//! Joystick keyboard hid event
struct usbh_joy_hid_event {
	uint32_t buttons;			//! Button bitmaps
	uint16_t X;					//! X axis
	uint16_t Y;					//! Y axis
	uint16_t Z;					//! Z axis
	uint16_t rX;				//! RX axis
	uint16_t rY;				//! RY axis
	uint16_t rZ;				//! Rz report
	uint16_t hat;				//! Hat switch
	uint16_t slider;			//! Slider value
	uint8_t n_buttons;			//! Number of buttons in report
	union {
		uint8_t has_bits;
		struct {
			uint8_t X: 1;			//! Has X report
			uint8_t Y: 1;			//! Has Y report
			uint8_t Z: 1;			//! Has Z report
			uint8_t rX: 1;			//!  Has RX in report
			uint8_t rY: 1;			//!  Has RY in report
			uint8_t rZ: 1;			//!  Has RZ in report
			uint8_t hat : 1;		//!  Has Hat in report
			uint8_t slider : 1;		//!  Has slider value
		} has;
	}; 
};
typedef struct usbh_joy_hid_event usbh_joy_hid_event_t;

//! Joystick structure operation
struct usbh_hid_joystick_ops {
	void ( *connected )( const usbh_hid_joy_context_t* id );			//New keyb event
	void ( *disconnected )( const usbh_hid_joy_context_t* id );		//Disconnect event
	void ( *report )( const usbh_hid_joy_context_t *id, const usbh_joy_hid_event_t* evt ); // Report event
	void (*enum_desc)( const usbh_hid_joy_context_t *id, enum usbh_driver_desc_type desc, const char *str );
};
typedef struct usbh_hid_joystick_ops usbh_hid_joystick_ops_t;
  
//! Initialize the joystick HID driver
const struct usbh_driver*
	usbh_hid_joystick_init( const usbh_hid_joystick_ops_t* joy_ops );
  
#ifdef __cplusplus
}
#endif
