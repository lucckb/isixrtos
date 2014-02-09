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
#include <usb/core/usbh_std_req.h>
#include <isix.h>
#include <stdlib.h>
/* ------------------------------------------------------------------ */ 
typedef struct usbh_keyb_hid_context {
	usbh_hid_context_t *hid;
} usbh_keyb_hid_context_t;
/* ------------------------------------------------------------------ */ 
//Keyboard operator struct
 static const usbh_hid_kbd_ops_t* g_kbd_ops;
/* ------------------------------------------------------------------ */
//Keyboard interface comparision for joystick
static int dcomp_keyboard_interface( const void* curr_desc)
{
	const usb_descriptor_header_t* hdr = DESCRIPTOR_PCAST( curr_desc, usb_descriptor_header_t );
	if( hdr->Type == INTERFACE_DESCRIPTOR ) {
		const usb_interface_descriptor_t* ifc = DESCRIPTOR_PCAST( curr_desc, usb_interface_descriptor_t );
		if( ifc->bInterfaceClass ==  HUMAN_INTERFACE_DEVICE_CLASS &&
			ifc->bInterfaceSubClass == BOOT_INTERFACE_SUBCLASS &&
			ifc->bInterfaceProtocol == KEYBOARD_PROTOCOL )
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

//FIXME: Test only kbd parsing and send queue should be done
static int new_keyboard_data;
static unsigned keyboard_modifiers;
static uint8_t keyboard_scan_code[KEYBOARD_MAX_PRESSED_KEYS];
static int num_lock, caps_lock;
static uint8_t last_report = 0;
static uint8_t report = 0;
static int send_rep_error = -1;


static void report_irq_callback( usbh_hid_context_t* ctx, const uint8_t* pbuf, uint8_t len )
{
	uint32_t i;
	(void)len;
	int new_num_lock, new_caps_lock;

	if( len == 0 ) {	//Disconnection event
		return;
	}
	for (i = 2; i < 2 + KEYBOARD_MAX_PRESSED_KEYS; ++i) {
		if (pbuf[i] == 1 || pbuf[i] == 2 || pbuf[i] == 3) {
			return ; /* error */
		}
	}
	keyboard_modifiers = pbuf[0];
	new_caps_lock = new_num_lock = 0;
	for (i = 0; i < KEYBOARD_MAX_PRESSED_KEYS; ++i) {
		keyboard_scan_code[i] = pbuf[i + 2];
		if (keyboard_scan_code[i] == NUM_LOCK_SCAN_CODE)
			new_num_lock = 1;
		if (keyboard_scan_code[i] == CAPS_LOCK_SCAN_CODE)
			new_caps_lock = 1;
	}
	new_keyboard_data = 1;

	if (num_lock == 0 && new_num_lock == 1)
		report ^= KEYBOARD_NUM_LOCK_LED;
	if (caps_lock == 0 && new_caps_lock == 1)
		report ^= KEYBOARD_CAPS_LOCK_LED;
	num_lock = new_num_lock;
	caps_lock = new_caps_lock;
	if( report != last_report ) {
		send_rep_error = usbh_hid_sent_report( ctx , &report, sizeof report );
		last_report = report;
	}
}
/* ------------------------------------------------------------------ */
/* Driver autodetection and attaching */
static int hid_keyboard_attached( const struct usbhost_device* hdev, void** data ) 
{
	const usb_device_descriptor_t* dev_desc = &hdev->dev_desc;
	const void* cdesc = hdev->cdesc;
	uint16_t cdsize = hdev->cdsize;
	//Validate device descriptor
	if ( dev_desc->bDeviceClass != 0 ||
		 dev_desc->bDeviceSubClass != 0 ||
		 dev_desc->bDeviceProtocol != 0 ) {
		dbprintf("Dev class not match");
		return usbh_driver_ret_not_found;
	}
	const usb_configuration_descriptor_t* cfg_desc = DESCRIPTOR_PCAST( cdesc, usb_configuration_descriptor_t );
	int ret = usb_get_next_descriptor_comp( &cdsize, &cdesc, dcomp_keyboard_interface ); 
	if ( ret != DESCRIPTOR_SEARCH_COMP_Found ) {
		dbprintf("Descriptor not match");
		return usbh_driver_ret_not_found;
	}
	const usb_interface_descriptor_t *if_desc = DESCRIPTOR_PCAST( cdesc , usb_interface_descriptor_t );
	dbprintf("CLASS %i SUBCLAS %i BOOT %i", if_desc->bInterfaceClass, if_desc->bInterfaceSubClass, if_desc->bInterfaceProtocol );
	//OK search for main hid descriptor
	ret = usb_get_next_descriptor_comp( &cdsize, &cdesc, dcomp_hid_desc ); 
	if ( ret != DESCRIPTOR_SEARCH_COMP_Found ) {
		return usbh_driver_ret_not_found;
	}
	//const usb_hid_main_descriptor_t* hid_desc = DESCRIPTOR_PCAST( cdesc, usb_hid_main_descriptor_t );
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
	/* Set hid boot protocol */
	ret = usbh_hid_set_boot_protocol( USBH_SYNC, if_desc->bInterfaceNumber, HID_BOOT_PROTOCOL );
	if (ret != USBHLIB_SUCCESS) {
		return ret;
	}
	/* Set infinity idle time. This request is optional for mouse. */
	ret = usbh_hid_set_idle( USBH_SYNC, if_desc->bInterfaceNumber, 0, 0 );
	if (ret != USBHLIB_SUCCESS &&
		(if_desc->bInterfaceProtocol != MOUSE_PROTOCOL || ret != USBHLIB_ERROR_STALL)) {
		return ret;
	}
	
	//Prepare keyboard context
	*data = malloc( sizeof( usbh_keyb_hid_context_t ) );
	if(!(*data)) {
		return USBHLIB_ERROR_NO_MEM;
	}
	usbh_keyb_hid_context_t* ctx = (usbh_keyb_hid_context_t*)*data;
	ctx->hid = usbh_hid_core_new_ctx();
	ret = usbh_hid_set_machine(ctx->hid, hdev->speed, hdev->dev_addr, 
			if_desc->bInterfaceNumber, 8,  ep_desc, if_desc->bNumEndpoints, report_irq_callback );
	if (ret != USBHLIB_SUCCESS) {
		return ret;
	}
	return usbh_driver_ret_configured;
}
/* ------------------------------------------------------------------ */ 
//! Process the hid keyboard 
static int hid_keyboard_process( void* data ) 
{
	usbh_keyb_hid_context_t* ctx = (usbh_keyb_hid_context_t*)data;
	dbprintf("Hid machine setup ok");
	while (usbh_hid_is_device_ready(ctx->hid)) {
			if( new_keyboard_data ) {
				new_keyboard_data = false;
				dbprintf("Keyboard modif %02x", keyboard_modifiers );
				dbprintf("Lrep %02x err %i", last_report, send_rep_error );
				for( int i=0;i<KEYBOARD_MAX_PRESSED_KEYS; ++i ) {
					dbprintf("Code %02x", keyboard_scan_code[i] );
				}
			}
		isix_wait_ms(25);
	}
	dbprintf("KBD or mouse disconnected err %i", usbh_hid_error(ctx->hid));
	free( ctx->hid );
	free( ctx );
	return USBHLIB_SUCCESS;
}
/* ------------------------------------------------------------------ */ 
//! Driver Ops structure
static const struct usbh_driver drv_ops = {
	attached:  hid_keyboard_attached,
	process:   hid_keyboard_process
};
/* ------------------------------------------------------------------ */ 
//Initialize core hid driver
const struct usbh_driver* usbh_hid_keyboard_init( const usbh_hid_kbd_ops_t* kbd_ops  ) 
{
	g_kbd_ops = kbd_ops;
	return &drv_ops;
}
/* ------------------------------------------------------------------ */ 

