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
#include <usb/host/usbh_driver.h>
#include <usb/host/controller.h>
#include <usb/host/internal.h>
#include <isix.h>
#include <isix/prv/list.h>
#include <foundation/sys/dbglog.h>
#include <stdlib.h>
#include <usb/host/usb_device_struct.h>

  
enum { OS_STACK_SIZE = 2048 };
enum { DEFAULT_CONFIGURATION = 1 };
  

//! USB controller context
struct usbhost_controller_context {
	ostask_t host_task;					//ISIX host base task
	list_entry_t usb_drivers;			//USB drivers list
	int err;							//Library error
	usbhost_device_t dev;				//Attached device
	const usbh_driver_t* drv;			//Current attached driver
	ossem_t lock;						//Driver lock semaphore
};

//! Driver item
struct usbhost_driver_item {
	const usbh_driver_t* drv;			//Driver struture
	bool in_use;						//Driver in use
	list_t inode;						//Driver add
};

  
//Only one controller is suppored now
static struct usbhost_controller_context ctx;

  
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
  
//Operating system usb task
static void usbhost_os_task( void* unused_os_arg )
{
	(void)unused_os_arg;
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

			// free config descriptor if previously allocated
			if (ctx.dev.cdesc) free(ctx.dev.cdesc);

			ctx.dev.cdsize = cfg_desc.wTotalLength;
			ctx.dev.cdesc = malloc( cfg_desc.wTotalLength );
			if( !ctx.dev.cdesc ) {
				ctx.err = USBHLIB_ERROR_NO_MEM;
				continue;
			}
			dbprintf("Total descriptor len %i", ctx.dev.cdsize );
			ctx.err = usbh_get_conf_descriptor(USBH_SYNC, 0, ctx.dev.cdesc, ctx.dev.cdsize );
			if (ctx.err != USBHLIB_SUCCESS) {
				dbprintf("usbh_get_conf_descriptor 2 err: %i", ctx.err );
				continue;
			}
			if( DESCRIPTOR_TYPE( ctx.dev.cdesc ) !=  CONFIGURATION_DESCRIPTOR )	{
				dbprintf("Unable to find valid config descriptor");
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
				ctx.drv = NULL;
				list_for_each_entry( &ctx.usb_drivers, i, inode ) {
					const int ret = i->drv->attached(&ctx.dev, &ctx.dev.data);
					if( ret == usbh_driver_ret_configured ) {
						ctx.drv = i->drv;
						// Internal driver error
						break;
					} else if( ret != usbh_driver_ret_not_found ) {
						// Internal driver error
						ctx.err = ret;
						break;
					}
				}

				if( !ctx.drv ) {
					ctx.err = USBHLIB_ERROR_NO_DRIVER;
					isix_sem_signal( ctx.lock );
					dbprintf("Device driver not found %i", ctx.err );
					continue;
				}
				if( ctx.drv->enum_desc ) {
					char sbuf[64] = { 0 };
					//Read string descriptor
					if( ctx.dev.dev_desc.iManufacturer ) {
						unsigned len = sizeof sbuf;
						ctx.err = usbh_get_string_descriptor_ascii(USBH_SYNC, 
								ctx.dev.dev_desc.iManufacturer, sbuf ,&len );
						if( ctx.err != USBHLIB_SUCCESS ) {
							dbprintf("Unable to read manufacturer string descriptor");
						} else {
							ctx.drv->enum_desc( ctx.dev.data, usbh_driver_desc_manufacturer, sbuf );
						}
					} 
					//Read device descriptor
					sbuf[0] = '\0';
					if( ctx.dev.dev_desc.iProduct ) {
						unsigned len = sizeof sbuf;
						ctx.err = usbh_get_string_descriptor_ascii(USBH_SYNC, 
								ctx.dev.dev_desc.iProduct, sbuf ,&len );
						if( ctx.err != USBHLIB_SUCCESS ) {
							dbprintf("Unable to read product string descriptor");
						} else {
							ctx.drv->enum_desc( ctx.dev.data, usbh_driver_desc_product, sbuf );
						}
					} 
					//Read serial descriptor
					sbuf[0] = '\0';
					if( ctx.dev.dev_desc.iSerialNumber ) {
						unsigned len = sizeof sbuf;
						ctx.err = usbh_get_string_descriptor_ascii(USBH_SYNC, 
								ctx.dev.dev_desc.iSerialNumber, sbuf ,&len );
						if( ctx.err != USBHLIB_SUCCESS ) {
							dbprintf("Unable to read iSerialNumber string descriptor");
						} else {
							ctx.drv->enum_desc( ctx.dev.data, usbh_driver_desc_serial, sbuf );
						}
					} 
				}
			}
			isix_sem_signal( ctx.lock );
			dbprintf("Process enter");
			ctx.err = ctx.drv->process( ctx.dev.data );
			dbprintf("Process exit");
		}
	} while(true);

	// free config descriptor
	if (ctx.dev.cdesc)
	{
		free(ctx.dev.cdesc);
		ctx.dev.cdesc = NULL;
	}
}

  
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
		ctx.host_task = isix_task_create( usbhost_os_task, NULL, OS_STACK_SIZE, os_priority,0);
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
			isix_task_kill( ctx.host_task );
		}
	}
	dbprintf( "Controller init with code: %i", ctx.err );
	return ctx.err;
}
  
// Get last library error
int usbh_controller_get_errno( void ) 
{
	return ctx.err;
}
  
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

 
//!Get Vendor ID
uint16_t usbh_get_id_vendor( void )
{
	return ctx.dev.dev_desc.idVendor;
}

 
//!Get Product ID
uint16_t usbh_get_id_product( void )
{
	return ctx.dev.dev_desc.idProduct;
}

  

