/*
 * =====================================================================================
 *
 *       Filename:  controller.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05.02.2014 22:22:50
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include "usbh_driver.h"
#include <usb/host/controller.h>
#include <usb/host/internal.h>
#include <isix.h>
#include <prv/list.h>
#include <foundation/dbglog.h>
#include <stdlib.h>
#include <usb/host/usb_device_struct.h>

/* ------------------------------------------------------------------ */ 
enum { OS_STACK_SIZE = 2048 };
enum { SHORT_TIMEOUT  = 250 };
enum { DEFAULT_CONFIGURATION = 1 };
/* ------------------------------------------------------------------ */ 

//! USB controller context
struct usbhost_controller_context {
	task_t* host_task;					//ISIX host base task
	list_entry_t usb_drivers;			//USB drivers list
	int err;							//Library error
	struct usbhost_device dev;			//Attached device
	struct usbh_driver* drv;			//Current attached driver
};

/* ------------------------------------------------------------------ */ 
//Only one controller is suppored now
static struct usbhost_controller_context ctx;

/* ------------------------------------------------------------------ */ 
//Operating system usb task
static void usbhost_os_task( void* ptr )
{
	(void)ptr;
	do {
		ctx.err = usbh_open_device(&ctx.dev.speed, &ctx.dev.dev_addr, &ctx.dev.dev_desc, ISIX_TIME_INFINITE);
		if( ctx.err != USBHLIB_SUCCESS ) {
			dbprintf("USB device open failed with code %i", ctx.err );
			break;
		}
		dbprintf("Something dev connected speed %i", ctx.dev.speed );
		//Second loop for device handling
		do {
			{
				usb_configuration_descriptor_t cfg_desc;
				ctx.err = usbh_get_conf_descriptor(USBH_SYNC, DEFAULT_CONFIGURATION, 
							(uint8_t *)&cfg_desc, sizeof(cfg_desc));
				if (ctx.err != USBHLIB_SUCCESS) {
					dbprintf("usbh_get_conf_descriptor %i", ctx.err );
					break;
				}
				ctx.dev.cfg_desc.size = cfg_desc.wTotalLength;
				ctx.dev.cfg_desc.mem = malloc( cfg_desc.wTotalLength );
				if( !ctx.dev.cfg_desc.mem ) {
					ctx.err = USBHLIB_ERROR_NO_MEM;
					break;
				}
				dbprintf("Total descriptor len %i", ctx.dev.cfg_desc.size );
				ctx.err = usbh_get_conf_descriptor(USBH_SYNC, 0, ctx.dev.cfg_desc.mem, ctx.dev.cfg_desc.size );
				if (ctx.err != USBHLIB_SUCCESS) {
					dbprintf("usbh_get_conf_descriptor 2 err: %i", ctx.err );
					break;
				}
			}
		} while( false );
	} while(true);
}
/* ------------------------------------------------------------------ */ 
//! Initialize the USB host controller
int usbh_controller_init( int controller_id , int os_priority )
{
	ctx.err = USBHLIB_SUCCESS;
	list_init( &ctx.usb_drivers );
	do {
		ctx.err = usbh_configure( controller_id );
		if( ctx.err != USBHLIB_SUCCESS ) {
			break;
		}
		ctx.host_task = isix_task_create( usbhost_os_task, NULL, OS_STACK_SIZE, os_priority );
		if( !ctx.host_task ) {
			ctx.err = USBHLIB_ERROR_OS;
			break;
		}
	} while(0);
	dbprintf( "Controller init with code: %i", ctx.err );
	return ctx.err;
}
/* ------------------------------------------------------------------ */ 
// Get last library error
int usbh_controller_get_errno( void ) 
{
	return ctx.err;
}
/* ------------------------------------------------------------------ */ 
