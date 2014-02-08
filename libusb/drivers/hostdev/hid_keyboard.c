/*
 * =====================================================================================
 *
 *       Filename:  hid_keyboard.h
 *
 *    Description:  Hid keyboard driver
 *
 *        Version:  1.0
 *        Created:  07.02.2014 23:08:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <usb/drivers/hostdev/hid_keyboard.h>
#include <usb/host/usbh_driver.h>
#include <foundation/dbglog.h>
#include <usb/core/usbh_config_descriptors.h>
#include <usb/core/usb_def.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_hid_req.h>
#include <usb/core/usbh_hid_core.h>
#include <usb/host/usb_device_struct.h>
#include <isix.h>
/* ------------------------------------------------------------------ */
//Keyboard interface comparision for joystick
static int dcomp_keyboard_interface( const void* curr_desc)
{
	const usb_descriptor_header_t* hdr = DESCRIPTOR_PCAST( curr_desc, usb_descriptor_header_t );
	if( hdr->Type == INTERFACE_DESCRIPTOR ) {
		const usb_interface_descriptor_t* ifc = DESCRIPTOR_PCAST( curr_desc, usb_interface_descriptor_t );
		if( ifc->bInterfaceClass ==  HUMAN_INTERFACE_DEVICE_CLASS &&
			ifc->bInterfaceSubClass == BOOT_INTERFACE_SUBCLASS ) {
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
		return DESCRIPTOR_SEARCH_Found;
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
/* Driver autodetection and attaching */
static int hid_keyboard_attached( const struct usbhost_device* hdev, void** data ) 
{
	(void)data;
	const usb_device_descriptor_t* dev_desc = &hdev->dev_desc;
	const void* cdesc = hdev->cdesc;
	uint16_t cdsize = hdev->cdsize;
	//Validate device descriptor
	if ( dev_desc->bDeviceClass != 0 ||
		 dev_desc->bDeviceSubClass != 0 ||
		 dev_desc->bDeviceProtocol != 0 )
	{
		return usbh_driver_ret_not_found;
	}
	int ret = usb_get_next_descriptor_comp( &cdsize, &cdesc, dcomp_keyboard_interface ); 
	if ( ret != DESCRIPTOR_SEARCH_COMP_Found ) {
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
	
	/* Set hid boot protocol */
	ret = usbh_hid_set_boot_protocol(1, if_desc->bInterfaceNumber, HID_BOOT_PROTOCOL);
	if (ret != USBHLIB_SUCCESS) {
		return ret;
	}
	/* Set infinity idle time. This request is optional for mouse. */
	ret = usbh_hid_set_idle(1, if_desc->bInterfaceNumber, 0, 0);
	if (ret != USBHLIB_SUCCESS &&
		(if_desc->bInterfaceProtocol != MOUSE_PROTOCOL || ret != USBHLIB_ERROR_STALL)) {
		return ret;
	}
	ret = usbh_hid_set_machine(hdev->speed, hdev->dev_addr, if_desc, hid_desc, ep_desc, if_desc->bNumEndpoints );
	if (ret != USBHLIB_SUCCESS) {
		return ret;
	}
	return usbh_driver_ret_configured;
}
/* ------------------------------------------------------------------ */ 
//! Process the hid keyboard 
static int hid_keyboard_process( void* data ) 
{
	(void)data;	
	dbprintf("Hid machine setup ok");
	while (usbh_hid_is_device_ready()) {
			if( new_keyboard_data ) {
				new_keyboard_data = false;
				dbprintf("Keyboard modif %02x", keyboard_modifiers );
				for( int i=0;i<KEYBOARD_MAX_PRESSED_KEYS; ++i ) {
					dbprintf("Code %02x", keyboard_scan_code[i] );
				}
			}
		if( new_mouse_data ) {
			dbprintf("Button: %02x x: %i y: %i", (unsigned)mouse_buttons, mouse_x, mouse_y );
			new_mouse_data = false;
		}
		isix_wait_ms(250);
	}
	dbprintf("KBD or mouse disconnected");
	return 0;
}
/* ------------------------------------------------------------------ */ 
//! Driver Ops structure
static const struct usbh_driver drv_ops = {
	attached:  hid_keyboard_attached,
	process:   hid_keyboard_process
};
/* ------------------------------------------------------------------ */ 
//Initialize core hid driver
const struct usbh_driver* usbh_hid_keyboard_init( void ) {
	return &drv_ops;
}
/* ------------------------------------------------------------------ */ 

