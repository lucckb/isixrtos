/*
 * =====================================================================================
 *
 *       Filename:  hid_joystick.c
 *
 *    Description:  HID joystick driver implementation
 *
 *        Version:  1.0
 *        Created:  22.02.2014 13:21:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <usb/drivers/hostdev/hid_joystick.h>
#include <usb/host/usbh_driver.h>
#include <foundation/dbglog.h>
#include <usb/core/usbh_config_descriptors.h>
#include <usb/core/usb_def.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_hid_req.h>
#include <usb/core/usbh_hid_core.h>
#include <usb/host/usb_device_struct.h>
#include <usb/core/usbh_std_req.h>
#include <usb/core/usb_endianness.h>
#include <isix.h>
#include <stdlib.h>
#include <string.h>
/* ------------------------------------------------------------------ */ 
//!Joystick operation structure
static const usbh_hid_joystick_ops_t* g_joy_ops;
/* ------------------------------------------------------------------ */ 
//!Joystick inteface comparator
static int dcomp_joystick_interface( const void* curr_desc )
{
	const usb_descriptor_header_t* hdr = DESCRIPTOR_PCAST( curr_desc, usb_descriptor_header_t );
	if( hdr->Type == INTERFACE_DESCRIPTOR ) {
		const usb_interface_descriptor_t* ifc = DESCRIPTOR_PCAST( curr_desc, usb_interface_descriptor_t );
		if( ifc->bInterfaceClass ==  HUMAN_INTERFACE_DEVICE_CLASS &&
			ifc->bInterfaceSubClass == 0 &&
			ifc->bInterfaceProtocol == 0 )
		{
				return DESCRIPTOR_SEARCH_Found;
		}
	}
	return DESCRIPTOR_SEARCH_NotFound;
}
/* ------------------------------------------------------------------ */ 
//Search for hid interface
static int dcomp_hid_desc( const void* curr_desc ) 
{
	const usb_descriptor_header_t* hdr = DESCRIPTOR_PCAST( curr_desc, usb_descriptor_header_t );
	if( hdr->Type == HID_MAIN_DESCRIPTOR ) {
		const usb_hid_main_descriptor_t* hid = DESCRIPTOR_PCAST( curr_desc, usb_hid_main_descriptor_t );
		if( USBTOHS( hid->bcdHID ) >= 0x100 ) {
			return DESCRIPTOR_SEARCH_Found;
		}
	}
	return DESCRIPTOR_SEARCH_NotFound;
}
/* ------------------------------------------------------------------ */
static int dcomp_endp_desc( const void* curr_desc )
{
	const usb_descriptor_header_t* hdr = DESCRIPTOR_PCAST( curr_desc, usb_descriptor_header_t );
	if( hdr->Type == ENDPOINT_DESCRIPTOR ) {
		return DESCRIPTOR_SEARCH_Found;
	}
	return DESCRIPTOR_SEARCH_NotFound;
}
/* ------------------------------------------------------------------ */
//! Driver auto detect and attach
static int hid_joystick_attached( const usbhost_device_t* hdev, void** data )
{
	const usb_device_descriptor_t* dev_desc = &hdev->dev_desc;
	const void* cdesc = hdev->cdesc;
	uint16_t cdsize = hdev->cdsize;
	//Validate device descriptor
	if ( dev_desc->bDeviceClass != 0 ||
		 dev_desc->bDeviceSubClass != 0 ||
		 dev_desc->bDeviceProtocol != 0 ) {

		if ( dev_desc->bDeviceClass != HUMAN_INTERFACE_DEVICE_CLASS ||
			dev_desc->bDeviceSubClass != 0 ||
			dev_desc->bDeviceProtocol != 0 ) {
			return usbh_driver_ret_not_found;
		}
	}
	//Test the configuration descriptor
	const usb_configuration_descriptor_t* cfg_desc = DESCRIPTOR_PCAST( cdesc, usb_configuration_descriptor_t );
	int ret = usb_get_next_descriptor_comp( &cdsize, &cdesc, dcomp_joystick_interface ); 
	if ( ret != DESCRIPTOR_SEARCH_COMP_Found ) {
		dbprintf("Descriptor not match");
		return usbh_driver_ret_not_found;
	}
	const usb_interface_descriptor_t *if_desc = DESCRIPTOR_PCAST( cdesc , usb_interface_descriptor_t );
	//OK search for main hid descriptor
	ret = usb_get_next_descriptor_comp( &cdsize, &cdesc, dcomp_hid_desc ); 
	if ( ret != DESCRIPTOR_SEARCH_COMP_Found ) {
		return usbh_driver_ret_not_found;
	}
	const usb_hid_main_descriptor_t* hid_desc = DESCRIPTOR_PCAST( cdesc, usb_hid_main_descriptor_t );
	//Find the endpoint descriptor
	ret = usb_get_next_descriptor_comp( &cdsize, &cdesc, dcomp_endp_desc );
	if ( ret != DESCRIPTOR_SEARCH_COMP_Found ) {
		return usbh_driver_ret_not_found;
	}
	const usb_endpoint_descriptor_t *ep_desc = DESCRIPTOR_PCAST (cdesc, usb_endpoint_descriptor_t );
	/* Set device configuration */
	ret = usbh_set_configuration( USBH_SYNC, cfg_desc->bConfigurationValue );
	if (ret != USBHLIB_SUCCESS) {
		return ret;
	}
	dbprintf("->>>>>>>>>> OKOKOK2 >>>>>>>>>> %02x", hid_desc->bcdHID );
	return usbh_driver_ret_not_found;
}
/* ------------------------------------------------------------------ */ 
//Enumerate descriptor
static void hid_joystick_enum_desc( void *data, enum usbh_driver_desc_type desc, const char *str )
{
	if( g_joy_ops && g_joy_ops->enum_desc ) {
		g_joy_ops->enum_desc( data, desc, str );
	}
}
/* ------------------------------------------------------------------ */ 
//! Driver keyboard process
static int hid_joystick_process( void* data )
{
	(void)data;
	return -1;
}
/* ------------------------------------------------------------------ */ 
//! Driver OPS structure
static const usbh_driver_t drv_ops = {
	hid_joystick_attached,
	hid_joystick_process,
	hid_joystick_enum_desc
};
/* ------------------------------------------------------------------ */ 
//Joystick Driver init
const struct usbh_driver*
	usbh_hid_joystick_init( const usbh_hid_joystick_ops_t* joy_ops )
{
	g_joy_ops = joy_ops;
	return &drv_ops;
}
/* ------------------------------------------------------------------ */ 
