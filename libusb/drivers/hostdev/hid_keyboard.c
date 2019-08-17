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
#include <foundation/sys/dbglog.h>
#include <usb/core/usbh_config_descriptors.h>
#include <usb/core/usb_def.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_hid_req.h>
#include <usb/core/usbh_hid_core.h>
#include <usb/host/usb_device_struct.h>
#include <usb/core/usbh_std_req.h>
#include <isix.h>
#include <stdlib.h>
#include <string.h>
  
static  const  uint8_t  hid_keybrd_codes[] = {
  0,     0,    0,    0,   31,   50,   48,   33, 
  19,   34,   35,   36,   24,   37,   38,   39,       /* 0x00 - 0x0F */
  52,    51,   25,   26,   17,   20,   32,   21,
  23,   49,   18,   47,   22,   46,    2,    3,       /* 0x10 - 0x1F */
  4,    5,    6,    7,    8,    9,   10,   11, 
  43,  110,   15,   16,   61,   12,   13,   27,       /* 0x20 - 0x2F */
  28,   29,   42,   40,   41,    1,   53,   54,  
  55,   30,  112,  113,  114,  115,  116,  117,       /* 0x30 - 0x3F */
  118,  119,  120,  121,  122,  123,  124,  125,  
  126,   75,   80,   85,   76,   81,   86,   89,       /* 0x40 - 0x4F */
  79,   84,   83,   90,   95,  100,  105,  106,
  108,   93,   98,  103,   92,   97,  102,   91,       /* 0x50 - 0x5F */
  96,  101,   99,  104,   45,  129,    0,    0, 
  0,    0,    0,    0,    0,    0,    0,    0,       /* 0x60 - 0x6F */
  0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,       /* 0x70 - 0x7F */
  0,    0,    0,    0,    0,  107,    0,   56,
  0,    0,    0,    0,    0,    0,    0,    0,       /* 0x80 - 0x8F */
  0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,       /* 0x90 - 0x9F */
  0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,       /* 0xA0 - 0xAF */
  0,    0,    0,    0,    0,    0,    0,    0, 
  0,    0,    0,    0,    0,    0,    0,    0,       /* 0xB0 - 0xBF */
  0,    0,    0,    0,    0,    0,    0,    0, 
  0,    0,    0,    0,    0,    0,    0,    0,       /* 0xC0 - 0xCF */
  0,    0,    0,    0,    0,    0,    0,    0, 
  0,    0,    0,    0,    0,    0,    0,    0,       /* 0xD0 - 0xDF */
  58,   44,   60,  127,   64,   57,   62,  128        /* 0xE0 - 0xE7 */
};
  
static  const  uint8_t  hid_keybrd_key[] = {
  '\0',  '`',  '1',  '2',  '3',  '4',  '5',  '6',
  '7',  '8',  '9',  '0',  '-',  '=',  '\0', '\x08',
  '\t',  'q',  'w',  'e',  'r',  't',  'y',  'u', 
  'i',  'o',  'p',  '[',  ']',  '\\',
  '\0',  'a',  's',  'd',  'f',  'g',  'h',  'j',  
  'k',  'l',  ';',  '\'', '\0', '\n',
  '\0',  '\0', 'z',  'x',  'c',  'v',  'b',  'n', 
  'm',  ',',  '.',  '/',  '\0', '\0',
  '\0',  '\0', '\0', ' ',  '\0', '\0', '\0', '\0', 
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
  '\0',  '\0', '\0', '\0', '\0', '\r', '\0', '\0', 
  '\0', '\0', '\0', '\0', '\0', '\0',
  '\0',  '\0', '7',  '4',  '1',
  '\0',  '/',  '8',  '5',  '2',
  '0',   '*',  '9',  '6',  '3',
  '.',   '-',  '+',  '\0', '\n', '\0', '\0', '\0', '\0', '\0', '\0',
  '\0',  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
  '\0', '\0', '\0', '\0'
};
  
static  const  uint8_t  hid_keybrd_shiftkey[] = {
  '\0', '~',  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',
  '_',  '+',  '\0', '\x08', '\0', 'Q',  'W',  'E',  'R',  'T',  'Y',  'U', 
  'I',  'O',  'P',  '{',  '}',  '|',  '\0', 'A',  'S',  'D',  'F',  'G', 
  'H',  'J',  'K',  'L',  ':',  '"',  '\0', '\n', '\0', '\0', 'Z',  'X',  
  'C',  'V',  'B',  'N',  'M',  '<',  '>',  '?',  '\0', '\0',  '\0', '\0',
  '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
  '\0', '\0', '\0', '\0', '\0', '\0', '\0',    '\0', '\0', '\0', '\0', '\0',
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'
};
  
//Keyboard context
typedef struct usbh_keyb_hid_context {
	usbh_hid_context_t* hid;		//Hid machine state context
	ossem_t report_sem;				//Notify semaphore
	usbh_keyb_hid_event_t evt;		//Keyboard event data
	bool disconnect;				//Disconnection event
	bool num_lock;					//Num lock
	bool caps_lock;					//Caps lock
	uint8_t last_report;			//Last report
	uint8_t report;					//Report 
	uint8_t nbr_keys_last;
	uint8_t keys_last[ KEYBOARD_MAX_PRESSED_KEYS ];
} usbh_keyb_hid_context_t;

  
//Keyboard operator struct
 static const usbh_hid_kbd_ops_t* g_kbd_ops;
 
//Keyboard interface comparision for joystick
static int dcomp_keyboard_interface( const void* curr_desc )
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
 
//Search for hid interface
static int dcomp_hid_desc( const void* curr_desc ) 
{
	const usb_descriptor_header_t* hdr = DESCRIPTOR_PCAST( curr_desc, usb_descriptor_header_t );
	if( hdr->Type == HID_MAIN_DESCRIPTOR ) {
		return DESCRIPTOR_SEARCH_Found;
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
 
/** HID machine state callback
 * @param[in] ctx HID context
 * @param[in] user_data Private user data
 * @param[in] pbuf Report buffer
 * @param[in] Report buffer len. If 0 disconnection event
 */
static void report_irq_callback( usbh_hid_context_t* hid,
		void* user_data, const uint8_t* pbuf, uint8_t len ) 
{
	usbh_keyb_hid_context_t* ctx = (usbh_keyb_hid_context_t*)user_data;
	bool new_keyboard_data = false;
	if( len == 0 ) {
		//Disconnection event
		ctx->disconnect = true;
		new_keyboard_data = true;
	} else {
		ctx->disconnect = false;
		if( KEYBOARD_MAX_PRESSED_KEYS + 2 != len ) {
			return;
		}
		//Detect ab error and exit without update
		for (int i = 2; i < 2 + KEYBOARD_MAX_PRESSED_KEYS; ++i) {
			if (pbuf[i] == 1 || pbuf[i] == 2 || pbuf[i] == 3) {
				return ; /* error */
			}
		}
		ctx->evt.scan_bits = pbuf[0];
		bool new_caps_lock = false;
		bool new_num_lock = false;
		int nbr_keys = 0;
		int nbr_keys_new = 0;
		uint8_t keys[ KEYBOARD_MAX_PRESSED_KEYS ];
		uint8_t keys_new[ KEYBOARD_MAX_PRESSED_KEYS ];
		for (int i = 0; i < KEYBOARD_MAX_PRESSED_KEYS; ++i) {
			uint8_t ckey = pbuf[i + 2];
			if ( ckey == NUM_LOCK_SCAN_CODE )
				new_num_lock = true;
			if ( ckey == CAPS_LOCK_SCAN_CODE )
				new_caps_lock = true;
			if( ckey != 0 ) {
				keys[nbr_keys++] = ckey;
				int j;
				for( j = 0; j<ctx->nbr_keys_last; ++j ) {
					if( ckey == ctx->keys_last[j] ) {
						break;
					}
				}
				if( j == ctx->nbr_keys_last ) {
					keys_new[ nbr_keys_new++ ] = ckey;
				}
			}
		}
		//Handle caps/num lock and outgoing report
		if (ctx->num_lock == false && new_num_lock == true)
			ctx->report ^= KEYBOARD_NUM_LOCK_LED;
		if (ctx->caps_lock == false && new_caps_lock == true )
			ctx->report ^= KEYBOARD_CAPS_LOCK_LED;
		ctx->num_lock = new_num_lock;
		ctx->caps_lock = new_caps_lock;
		if( ctx->report != ctx->last_report ) {
			usbh_hid_sent_report( hid , &ctx->report, sizeof ctx->report );
			ctx->last_report = ctx->report;
		}
		//Translate scan codes to keys
		if( nbr_keys_new == 1 ) {
			uint8_t sck = keys_new[0];
			uint8_t cap_letter = 
				(ctx->evt.scan_bits&(usbh_keyb_hid_scan_bit_l_shift|usbh_keyb_hid_scan_bit_r_shift))?1:0;
			cap_letter ^= ( ctx->report & KEYBOARD_CAPS_LOCK_LED )?1:0; 
			if( cap_letter ) {
				ctx->evt.key =  hid_keybrd_shiftkey[hid_keybrd_codes[sck]];
			} else {
				ctx->evt.key = hid_keybrd_key[hid_keybrd_codes[sck]];
			}
			ctx->evt.scan_code = sck;
		} else {
			ctx->evt.scan_code = 0;
			ctx->evt.key = 0;
		}
		new_keyboard_data = true;
		ctx->nbr_keys_last = nbr_keys;
		memcpy( ctx->keys_last, keys, sizeof keys );
	}
	if( new_keyboard_data ) {
		isix_sem_signal_isr( ctx->report_sem );
	}
}
 
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
	memset( ctx, 0, sizeof(usbh_keyb_hid_context_t) );
	//Setup the report sem
	ctx->report_sem = isix_sem_create_limited( NULL, 0, 1 );
	if( !ctx->report_sem ) {
		free( ctx );
		*data = NULL;
		return USBHLIB_ERROR_OS;
	}
	ctx->hid = usbh_hid_core_new_ctx();
	ret = usbh_hid_set_machine(ctx->hid, hdev->speed, hdev->dev_addr, 
			if_desc->bInterfaceNumber, 8,  ep_desc, if_desc->bNumEndpoints, report_irq_callback , ctx );
	if (ret != USBHLIB_SUCCESS) {
		isix_sem_destroy( ctx->report_sem );
		free( ctx );
		*data = NULL;
		return ret;
	}
	if( g_kbd_ops && g_kbd_ops->connected ) {
		g_kbd_ops->connected( ctx );
	}
	return usbh_driver_ret_configured;
}
  
static int hid_keyboard_process( void* data ) 
{
	usbh_keyb_hid_context_t* ctx = (usbh_keyb_hid_context_t*)data;
	while( true ) {
		if( isix_sem_wait( ctx->report_sem, ISIX_TIME_INFINITE ) != ISIX_EOK ) {
			return USBHLIB_ERROR_OS;
		}
		//New event arrived
		if( ctx->disconnect ) {
			if( g_kbd_ops && g_kbd_ops->disconnected ) {
				g_kbd_ops->disconnected( ctx );
			}
			free( ctx->hid );
			isix_sem_destroy( ctx->report_sem );
			free( ctx );
			return USBHLIB_SUCCESS;
		} else {	//Normal item
			if( g_kbd_ops && g_kbd_ops->report ) {
				g_kbd_ops->report( ctx , &ctx->evt );
			}
		}
	}
}

  
//! Handle string descriptor
static void hid_keyboard_enum_desc( void *data, enum usbh_driver_desc_type desc, const char *str )
{
	if( g_kbd_ops && g_kbd_ops->enum_desc ) {
		g_kbd_ops->enum_desc( data, desc, str );
	}
}
  
//! Driver Ops structure
static const usbh_driver_t drv_ops = {
	attached:  hid_keyboard_attached,
	process:   hid_keyboard_process,
	enum_desc: hid_keyboard_enum_desc
};
  
//Initialize core hid driver
const struct usbh_driver* usbh_hid_keyboard_init( const usbh_hid_kbd_ops_t* kbd_ops  ) 
{
	g_kbd_ops = kbd_ops;
	return &drv_ops;
}
  

