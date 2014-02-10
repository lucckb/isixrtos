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
/* ------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------ */ 
#include <stdint.h>
#include <stdbool.h>
/* ------------------------------------------------------------------ */ 
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

struct usbh_keyb_hid_event {
	usbh_keyb_hid_context_t* kbd_id;	//! Keyboard initializer
	uint8_t keys[6];					//! Translated key to UE char
	uint8_t scan_bits;					//! Special scan bits keys
	uint8_t n_scan_codes;				//! Number of scan codes
	uint8_t scan_codes[6];				//! Scan code extra key
};
typedef struct usbh_keyb_hid_event usbh_keyb_hid_event_t;

//! USB keyboard device operations
struct usbh_hid_kbd_ops {
	void ( *connected )( usbh_keyb_hid_context_t* id );			//New keyb event
	void ( *disconnected )( usbh_keyb_hid_context_t* id );		//Disconnect even	t
	void ( *report )( usbh_keyb_hid_context_t *id, const usbh_keyb_hid_event_t* evt ); // Report event
};
typedef struct usbh_hid_kbd_ops usbh_hid_kbd_ops_t; 
/* ------------------------------------------------------------------ */ 
//Initialize core hid driver
const struct usbh_driver* 
	usbh_hid_keyboard_init( const usbh_hid_kbd_ops_t* kbd_ops );

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
}
#endif
/* ------------------------------------------------------------------ */ 

