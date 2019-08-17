/*
 * =====================================================================================
 *
 *       Filename:  hid_keyboard.h
 *
 *    Description:  Hid keyboard host driver
 *
 *        Version:  1.0
 *        Created:  07.02.2014 23:10:05
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
  
#include <stdint.h>
#include <stdbool.h>
#include <usb/host/usbh_driver_desc_type.h>
  
struct usbh_driver;
struct usbh_keyb_hid_context;
typedef struct usbh_keyb_hid_context usbh_keyb_hid_context_t;

enum usbh_keyb_hid_scan_bits_enum {
	usbh_keyb_hid_scan_bit_l_ctrl = 	0x01,
	usbh_keyb_hid_scan_bit_l_shift = 	0x02,
	usbh_keyb_hid_scan_bit_l_alt = 		0x04,
	usbh_keyb_hid_scan_bit_l_gui = 		0x08,
	usbh_keyb_hid_scan_bit_r_ctrl = 	0x10,
	usbh_keyb_hid_scan_bit_r_shift = 	0x20,
	usbh_keyb_hid_scan_bit_r_alt = 		0x40,
	usbh_keyb_hid_scan_bit_r_gui = 		0x80
};
/* Connect disconnect keyboard event
 * if key == 0 it means that code is not decoded or
 * button is released
 * if scan_code = 0 it means that button is released
*/
struct usbh_keyb_hid_event {
	usbh_keyb_hid_context_t* kbd_id;	//! Keyboard initializer
	uint8_t key;						//! Translated key to UE char
	uint8_t scan_bits;					//! Special scan bits keys
	uint8_t scan_code;					//! Scan code extra key
};
typedef struct usbh_keyb_hid_event usbh_keyb_hid_event_t;

//! USB keyboard device operations
struct usbh_hid_kbd_ops {
	void ( *connected )( const usbh_keyb_hid_context_t* id );			//New keyb event
	void ( *disconnected )( const usbh_keyb_hid_context_t* id );		//Disconnect event
	void ( *report )( const usbh_keyb_hid_context_t *id, const usbh_keyb_hid_event_t* evt ); // Report event
	void (*enum_desc)( const usbh_keyb_hid_context_t* id, enum usbh_driver_desc_type desc, const char *str );
};
typedef struct usbh_hid_kbd_ops usbh_hid_kbd_ops_t; 
  
//Initialize core hid driver
const struct usbh_driver* 
	usbh_hid_keyboard_init( const usbh_hid_kbd_ops_t* kbd_ops );

 
#ifdef __cplusplus
}
#endif
  

