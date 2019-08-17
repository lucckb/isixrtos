/*
 * =====================================================================================
 *
 *       Filename:  usbh_driver_desc_type.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  18.02.2014 21:47:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

  
//! Descriptor of input request
enum usbh_driver_desc_type {
	usbh_driver_desc_product,
	usbh_driver_desc_manufacturer,
	usbh_driver_desc_serial
};
  
