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
	const struct usbh_driver* drv;		//Current attached driver
	sem_t* lock;						//Driver lock semaphore
};

//! Driver item
struct usbhost_driver_item {
	const struct usbh_driver* drv;		//Driver struture
	bool in_use;						//Driver in use
	list_t inode;						//Driver add
};

/* ------------------------------------------------------------------ */ 
//Only one controller is suppored now
static struct usbhost_controller_context ctx;

/* ------------------------------------------------------------------ */ 
//! Find the device driver by ptr
static struct usbhost_driver_item* find_driver( const struct usbh_driver* drv )
{
	if( !drv ) {
		return NULL;
	}
	struct usbhost_driver_item* i;
	list_for_each_entry( &ctx.usb_drivers, i, inode ) {
		if( i->drv == drv ) {
			return i;
		}
	}
	return NULL;
}
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
		{
			usb_configuration_descriptor_t cfg_desc;
			ctx.err = usbh_get_conf_descriptor(USBH_SYNC, DEFAULT_CONFIGURATION, 
						(uint8_t *)&cfg_desc, sizeof(cfg_desc));
			if (ctx.err != USBHLIB_SUCCESS) {
				dbprintf("usbh_get_conf_descriptor %i", ctx.err );
				continue;
			}
			ctx.dev.cfg_desc.size = cfg_desc.wTotalLength;
			ctx.dev.cfg_desc.mem = malloc( cfg_desc.wTotalLength );
			if( !ctx.dev.cfg_desc.mem ) {
				ctx.err = USBHLIB_ERROR_NO_MEM;
				continue;
			}
			dbprintf("Total descriptor len %i", ctx.dev.cfg_desc.size );
			ctx.err = usbh_get_conf_descriptor(USBH_SYNC, 0, ctx.dev.cfg_desc.mem, ctx.dev.cfg_desc.size );
			if (ctx.err != USBHLIB_SUCCESS) {
				dbprintf("usbh_get_conf_descriptor 2 err: %i", ctx.err );
				continue;
			}
			dbprintf("After full descriptor read");
			//Find the valid driver for device
			ctx.err = isix_sem_wait(ctx.lock, ISIX_TIME_INFINITE );
			if( ctx.err != ISIX_EOK ) {
				ctx.err = USBHLIB_ERROR_OS;
				break;
			}
			{
				struct usbhost_driver_item* i;
				list_for_each_entry( &ctx.usb_drivers, i, inode ) {
					if( i->drv->attached( &ctx.dev.cfg_desc ) == usbh_driver_ret_configured ) {
						ctx.drv = i->drv;
						break;
					}
				}
				if( !ctx.drv ) {
					dbprintf("Device driver not found for selected dev");
					break;
				}
			}
			isix_sem_signal( ctx.lock );
			ctx.err = ctx.drv->process( ctx.drv->data );
		}
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
		ctx.lock = isix_sem_create_limited( NULL, 1, 1 );
		if( !ctx.lock ) {
			ctx.err = USBHLIB_ERROR_OS;
			break;
		}
		ctx.host_task = isix_task_create( usbhost_os_task, NULL, OS_STACK_SIZE, os_priority );
		if( !ctx.host_task ) {
			ctx.err = USBHLIB_ERROR_OS;
			break;
		}
	} while(0);
	//If something failed destroy allocated resources
	if( ctx.err != USBHLIB_SUCCESS ) {
		if( ctx.lock ) {
			isix_sem_destroy( ctx.lock );
		}
		if( ctx.host_task ) {
			isix_task_delete( ctx.host_task );
		}
	}
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
//! Register USBHost driver
int usbh_controller_attach_driver( const struct usbh_driver *drv )
{
	if( !drv ) {
		return USBHLIB_ERROR_INVALID_PARAM;
	}
	if( !drv->process || !drv->attached ) {
		return USBHLIB_ERROR_INVALID_PARAM;
	}
	if( isix_sem_wait( ctx.lock, ISIX_TIME_INFINITE ) != ISIX_EOK ) {
		return USBHLIB_ERROR_OS;
	}
	if( find_driver( drv ) ) {
		isix_sem_signal( ctx.lock );
		return USBHLIB_ERROR_EXISTS;
	}
	struct usbhost_driver_item* it = malloc( sizeof( struct usbhost_driver_item ) );
	if( !it ) {
		isix_sem_signal( ctx.lock );
		return USBHLIB_ERROR_NO_MEM;
	}
	it->drv = drv;
	it->in_use = false;
	list_insert_end( &ctx.usb_drivers, &it->inode );
	isix_sem_signal_isr( ctx.lock );
	return USBHLIB_SUCCESS;
}

/* ------------------------------------------------------------------ */
//!Detach driver from host
int usbh_controller_detach_driver( const struct usbh_driver *drv )
{
	if( !drv ) {
		return USBHLIB_ERROR_INVALID_PARAM;
	}
	if( isix_sem_wait( ctx.lock, ISIX_TIME_INFINITE ) != ISIX_EOK ) {
		return USBHLIB_ERROR_OS;
	}
	struct usbhost_driver_item* item = find_driver( drv );
	if( item->in_use ) {
		isix_sem_signal( ctx.lock );
		return USBHLIB_ERROR_BUSY;
	}
	//Free resource
	//TODO: Eventualy add detach fn
	list_delete( &item->inode );
	free( item );
	isix_sem_signal( ctx.lock );
	return USBHLIB_SUCCESS;
}
/* ------------------------------------------------------------------ */ 

