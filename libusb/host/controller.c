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
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */ 
enum { OS_STACK_SIZE = 2048 };
/* ------------------------------------------------------------------ */ 
static task_t* host_task;	//ISIX host base task
/* ------------------------------------------------------------------ */ 
//Operating system usb task
static void usbhost_os_task( void* ptr )
{
	(void)ptr;
}
/* ------------------------------------------------------------------ */ 
//! Initialize the USB host controller
int usbh_controller_init( int controller_id ) 
{
	int ret = USBHLIB_SUCCESS;
	do {
		ret = usbh_configure( controller_id );
		if( ret != USBHLIB_SUCCESS ) {
			break;
		}
		host_task = isix_task_create( usbhost_os_task, NULL, OS_STACK_SIZE, isix_get_min_priority() );
		if( !host_task ) {
			ret = USBHLIB_ERROR_OS;
			break;
		}
	} while(0);
	return ret;
}
/* ------------------------------------------------------------------ */ 
