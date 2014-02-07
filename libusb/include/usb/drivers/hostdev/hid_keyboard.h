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
/* ------------------------------------------------------------------ */-
#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------ */ 
#include <stdint.h>
/* ------------------------------------------------------------------ */ 
//Initialize core hid driver
const struct usbh_driver* usbh_hid_keyboard_init( void );

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
}
#endif
/* ------------------------------------------------------------------ */ 

