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

/* ------------------------------------------------------------------ */ 
struct usbh_driver;
struct usbh_keyb_hid_context;
typedef struct usbh_keyb_hid_context usbh_keyb_hid_context_t;

//! USB keyboard device operations
struct usbh_hid_kbd_ops {
	void ( *connected )( usbh_keyb_hid_context_t* id );
	void ( *disconnected )( usbh_keyb_hid_context_t* id );
	void ( *report )( usbh_keyb_hid_context_t *id, int code );
};
typedef struct usbh_hid_kbd_ops usbh_hid_kbd_ops_t; 
/* ------------------------------------------------------------------ */ 
//Initialize core hid driver
const struct usbh_driver* usbh_hid_keyboard_init( const usbh_hid_kbd_ops_t* kbd_ops );

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
}
#endif
/* ------------------------------------------------------------------ */ 

