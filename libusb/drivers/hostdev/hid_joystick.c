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
#include <foundation/sys/dbglog.h>
#include <usb/core/usbh_config_descriptors.h>
#include <usb/core/usb_def.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_hid_req.h>
#include <usb/core/usbh_hid_core.h>
#include <usb/host/usb_device_struct.h>
#include <usb/core/usbh_std_req.h>
#include <usb/core/usb_endianness.h>
#include <usb/core/hid_parser.h>
#include <isix.h>
#include <stdlib.h>
#include <string.h>

  
//HID joystick specific constants
enum { USAGE_PAGE_BUTTON       =  0x09 };
/** HID Report Descriptor Usage Page value for a Generic Desktop Control. */
enum { USAGE_PAGE_GENERIC_DCTRL    = 0x01 };
/** HID Report Descriptor Usage for a Joystick. */
enum { USAGE_JOYSTICK              = 0x04 };
/** HID Report Descriptor Usage value for a X axis movement. */
enum { USAGE_X                     = 0x30 };
/** HID Report Descriptor Usage value for a Y axis movement. */
enum { USAGE_Y                     = 0x31 };
/** HID Report Descriptor Usage value for a Y axis movement. */
enum { USAGE_Z                     = 0x32 };
/** HID Report usage for Rx movement */
enum { USAGE_RX						= 0x33 };
/** HID Report usage for Rx movement */
enum { USAGE_RY						= 0x34 };
/** HID Report usage for Rx movement */
enum { USAGE_RZ						= 0x35 };
/** HID Report usage for Rx movement */
enum { USAGE_SLIDER					= 0x36 };
/** HID Report usage for Hat switch */
enum { USAGE_HAT_SW					= 0x39 };
  
//!Joystick operation structure
static const usbh_hid_joystick_ops_t* g_joy_ops;
 
//! Joystick context
typedef struct usbh_hid_joy_context {
	HID_ReportInfo_t hid_info;	//! HID report info
	ossem_t report_sem;			//! Report semaphore notifier
	usbh_hid_context_t* hidm;	//! HID machine context state
	bool disconnect;			//! It is disconnection event
	usbh_joy_hid_event_t evt;	//! Event data processed
} usbh_hid_joy_context_t;
  
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
 
static int dcomp_endp_desc( const void* curr_desc )
{
	const usb_descriptor_header_t* hdr = DESCRIPTOR_PCAST( curr_desc, usb_descriptor_header_t );
	if( hdr->Type == ENDPOINT_DESCRIPTOR ) {
		return DESCRIPTOR_SEARCH_Found;
	}
	return DESCRIPTOR_SEARCH_NotFound;
}
  
//! Joystick HID parser filter
static bool dcomp_hid_joystick( HID_ReportItem_t* const CurrentItem ) 
{

	bool IsJoystick = false;

	/* Iterate through the item's collection path, until either the root collection node or a collection with the
	 * Joystick Usage is found - this prevents Mice, which use identical descriptors except for the Mouse usage
	 * parent node, from being erroneously treated as a joystick
	 */
	for (HID_CollectionPath_t* CurrPath = CurrentItem->CollectionPath; 
			CurrPath != NULL; CurrPath = CurrPath->Parent)
	{
		if ((CurrPath->Usage.Page  == USAGE_PAGE_GENERIC_DCTRL) &&
		    (CurrPath->Usage.Usage == USAGE_JOYSTICK))
		{
			IsJoystick = true;
			break;
		}
	}

	/* If a collection with the joystick usage was not found, indicate that we are not interested in this item */
	if (!IsJoystick)
	  return false;

	/* Check the attributes of the current joystick item - see if we are interested in it or not;
	 * only store BUTTON and GENERIC_DESKTOP_CONTROL items into the Processed HID Report
	 * structure to save RAM and ignore the rest
	 */
	return ((CurrentItem->Attributes.Usage.Page == USAGE_PAGE_BUTTON) ||
	        (CurrentItem->Attributes.Usage.Page == USAGE_PAGE_GENERIC_DCTRL));
}
  
/** HID machine state callback
 * @param[in] ctx HID context
 * @param[in] user_data Private user data
 * @param[in] pbuf Report buffer
 * @param[in] Report buffer len. If 0 disconnection event
 */
static void report_irq_callback( usbh_hid_context_t* hid,
		void* user_data, const uint8_t* pbuf, uint8_t len ) 
{
	(void)hid;
	usbh_hid_joy_context_t* ctx = (usbh_hid_joy_context_t*)user_data;
	bool new_data = false;
	if( len == 0 ) {	//! Disconnection event
		ctx->disconnect = true;
		new_data = true;
	} else {	// Process the rest of data
		ctx->evt.n_buttons = 0;
		ctx->evt.buttons = 0;
		ctx->evt.has_bits = 0;
		ctx->evt.has_bits = 0;
		for( size_t report_no = 0; report_no < ctx->hid_info.TotalReportItems; ++report_no ) {
			HID_ReportItem_t* report_it = &ctx->hid_info.ReportItems[report_no];
			bool found = false;
			if( report_it->Attributes.Usage.Page == USAGE_PAGE_BUTTON &&
				report_it->ItemType == HID_REPORT_ITEM_In ) {
				found = usbh_hid_get_report_item_info( pbuf, report_it );
				if(!(found)) {
					continue;
				}
				//! Pass the button info
				ctx->evt.buttons |= report_it->Value << ctx->evt.n_buttons++;
				new_data = true;
			}
			else if ((report_it->Attributes.Usage.Page   == USAGE_PAGE_GENERIC_DCTRL) &&
				 	((report_it->Attributes.Usage.Usage == USAGE_X)                  ||
				 	(report_it->Attributes.Usage.Usage == USAGE_Y)                   ||
				 	(report_it->Attributes.Usage.Usage == USAGE_Z)                   ||
				 	(report_it->Attributes.Usage.Usage == USAGE_RZ)                  ||
				 	(report_it->Attributes.Usage.Usage == USAGE_HAT_SW)              ||
					(report_it->Attributes.Usage.Usage == USAGE_SLIDER)              ||
				 	(report_it->Attributes.Usage.Usage == USAGE_RX)                  ||
				  	(report_it->Attributes.Usage.Usage == USAGE_RY))                 &&
				 	(report_it->ItemType                == HID_REPORT_ITEM_In)) {
				found = usbh_hid_get_report_item_info( pbuf, report_it );
				if(!(found)) {
					continue;
				}
				uint16_t value = HID_ALIGN_DATA(report_it, uint16_t);
				if( report_it->Attributes.Usage.Usage == USAGE_X ) {
					ctx->evt.X = value;
					ctx->evt.has.X = 1;
				} else if( report_it->Attributes.Usage.Usage == USAGE_Y ) {
					ctx->evt.Y = value;
					ctx->evt.has.Y = 1;
				} else if( report_it->Attributes.Usage.Usage == USAGE_Z ) {
					ctx->evt.Z = value;
					ctx->evt.has.Z = 1;
				} else if( report_it->Attributes.Usage.Usage == USAGE_RX ) {
					ctx->evt.rX = value;
					ctx->evt.has.rX = 1;
				} else if( report_it->Attributes.Usage.Usage == USAGE_RY ) {
					ctx->evt.rY = value;
					ctx->evt.has.rY = 1;
				} else if ( report_it->Attributes.Usage.Usage == USAGE_RZ ) {
					ctx->evt.rZ = value;
					ctx->evt.has.rZ = 1;
				} else if( report_it->Attributes.Usage.Usage == USAGE_HAT_SW ) {
					// empirical fix for hat switch value					
					if (report_it->Value >= report_it->Attributes.Logical.Minimum)
						ctx->evt.hat = report_it->Value - report_it->Attributes.Logical.Minimum;
					else
						ctx->evt.hat = 0x0f;

//					ctx->evt.hat = report_it->Value;
					ctx->evt.has.hat = 1;
				} else if( report_it->Attributes.Usage.Usage == USAGE_SLIDER ) {
					ctx->evt.slider = value;
					ctx->evt.has.slider = 1;
				}
				new_data = true;
			}

		}
	}
	if( new_data ) {
		isix_sem_signal_isr( ctx->report_sem );
	}
}
 
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
		dbprintf("HID descriptor not found");
		return usbh_driver_ret_not_found;
	}
	const usb_hid_main_descriptor_t* hid_desc = DESCRIPTOR_PCAST( cdesc, usb_hid_main_descriptor_t );
	//Find the endpoint descriptor
	ret = usb_get_next_descriptor_comp( &cdsize, &cdesc, dcomp_endp_desc );
	if ( ret != DESCRIPTOR_SEARCH_COMP_Found ) {
		dbprintf("Endpoint descriptor not found");
		return usbh_driver_ret_not_found;
	}
	const usb_endpoint_descriptor_t *ep_desc = DESCRIPTOR_PCAST (cdesc, usb_endpoint_descriptor_t );
	/* Set device configuration */
	ret = usbh_set_configuration( USBH_SYNC, cfg_desc->bConfigurationValue );
	if (ret != USBHLIB_SUCCESS) {
		dbprintf("Unable to set device configuration");
		return ret;
	}
	*data = malloc( sizeof(usbh_hid_joy_context_t) );
	if(!(*data)) {
		dbprintf("No memory");
		return USBHLIB_ERROR_NO_MEM;
	}
	memset( *data, 0, sizeof(usbh_hid_joy_context_t) );
	usbh_hid_joy_context_t* ctx = (usbh_hid_joy_context_t*)*data;
	{
		uint16_t hlen = USBTOHS(hid_desc->wDescriptorLength1);
		uint8_t buf[hlen];
		ret = usbh_hid_get_report_descriptor( USBH_SYNC, buf, hlen );
		if( ret != USBHLIB_SUCCESS ) {
			dbprintf("Unable to get report descriptor");
			free( *data );
			*data = NULL;
			return ret;
		}
		ret = usbh_hid_process_report( buf, hlen, &ctx->hid_info, dcomp_hid_joystick );
		if( ret != HID_PARSE_Successful ) {
			dbprintf(" hid_process_report fail - Probably not a joystick, ret=%d", ret );
			free( *data );
			*data = NULL;
			return ret;
		}
	}
	if( ctx->hid_info.TotalReportItems == 0 ) {
		dbprintf("Not a valid joystick invalid report item count");
		free ( *data );
		*data = NULL;
		return ret;
	}
	//Setup the report sem
	ctx->report_sem = isix_sem_create_limited( NULL, 0, 1 );
	if( !ctx->report_sem ) {
		free( ctx );
		*data = NULL;
		dbprintf("Unable to create semaphore");
		return USBHLIB_ERROR_OS;
	}
	ctx->hidm = usbh_hid_core_new_ctx();
	ret = usbh_hid_set_machine(ctx->hidm, hdev->speed, hdev->dev_addr, 
			if_desc->bInterfaceNumber, 4 ,  ep_desc, if_desc->bNumEndpoints, report_irq_callback , ctx );
	if (ret != USBHLIB_SUCCESS) {
		isix_sem_destroy( ctx->report_sem );
		free( ctx );
		*data = NULL;
		dbprintf("Unable to set hid machine code %i", ret );
		return ret;
	}
	if( g_joy_ops && g_joy_ops->connected ) {
		g_joy_ops->connected( ctx );
	}
	return usbh_driver_ret_configured;
}
  
//Enumerate descriptor
static void hid_joystick_enum_desc( void *data, enum usbh_driver_desc_type desc, const char *str )
{
	if( g_joy_ops && g_joy_ops->enum_desc ) {
		g_joy_ops->enum_desc( data, desc, str );
	}
}
  
//! Driver keyboard process
static int hid_joystick_process( void* data )
{
	usbh_hid_joy_context_t* ctx = (usbh_hid_joy_context_t*)data;
	while( true ) {
		if( isix_sem_wait( ctx->report_sem, ISIX_TIME_INFINITE ) != ISIX_EOK ) {
			return USBHLIB_ERROR_OS;
		}
		if( ctx->disconnect ) {	//! Disconnection event
			if( g_joy_ops && g_joy_ops->disconnected ) {
				g_joy_ops->disconnected( ctx );
			}
			free( ctx->hidm );
			isix_sem_destroy( ctx->report_sem );
			free ( ctx );
			return USBHLIB_SUCCESS;
		} else { 	// Normal item
			if( g_joy_ops && g_joy_ops->report ) {
				g_joy_ops->report( ctx, &ctx->evt );
			}
		}
	}
}
  
//! Driver OPS structure
static const usbh_driver_t drv_ops = {
	hid_joystick_attached,
	hid_joystick_process,
	hid_joystick_enum_desc
};
  
//Joystick Driver init
const struct usbh_driver*
	usbh_hid_joystick_init( const usbh_hid_joystick_ops_t* joy_ops )
{
	g_joy_ops = joy_ops;
	return &drv_ops;
}
  
