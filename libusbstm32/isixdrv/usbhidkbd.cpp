/*
 * usbhost.c
 *
 *  Created on: 20-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include "usbh_hid_keybd.h"
#include <usbh_core.h>
#include <usbh_hid_core.h>
#include <usb_hcd_int.h>
#include <dbglog.h>
#include <isix.h>
#include <isix/dev/input.hpp>
#include <usbhidkbd.hpp>
/* ------------------------------------------------------------------ */
//USB host core handle
static USB_OTG_CORE_HANDLE    usb_otg_dev;
static USBH_HOST usb_host;

namespace {
namespace usb_usr_cb {
	void init(void)
	{
		dbprintf("Init");
	}
	void deinit(void)
	{
		dbprintf("Init");
	}
	void attached(void)
	{
		dbprintf("Device attached");
	}
	void reset(void)
	{
		dbprintf("Device reset");
	}

	void disconnected(void)
	{
		dbprintf("Device disconnected");
	}
	void overcurrent (void)
	{
		dbprintf("Overcurrent detected.\n");

	}

	void speed_detected(uint8_t DeviceSpeed)
	{
	  if(DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED)
	  {
		  dbprintf("HPRT0_PRTSPD_HIGH_SPEED.");
	  }
	  else if(DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
	  {
		  dbprintf("HPRT0_PRTSPD_FULL_SPEED.");
	  }
	  else if(DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
	  {
		  dbprintf("HPRT0_PRTSPD_LOW_SPEED.");
	  }
	  else
	  {
		  dbprintf("Speed error");
	  }
	}

	void desc_available(void *DeviceDesc)
	{
	  const USBH_DevDesc_TypeDef *hs = reinterpret_cast<const USBH_DevDesc_TypeDef*>(DeviceDesc);
	  dbprintf( "VID : %04Xh\n" , (*hs).idVendor);
	  dbprintf( "PID : %04Xh\n" , (*hs).idProduct);
	}


	void device_address_assigned(void)
	{
		dbprintf("Dev address assigned ");
	}

	const uint16_t KYBRD_FIRST_COLUMN        =      319;
	const uint16_t KYBRD_LAST_COLUMN         =      7;
	const uint16_t KYBRD_FIRST_LINE          =      120;
	const uint16_t KYBRD_LAST_LINE           =      200;



	/**
	* @brief  USBH_USR_Conf_Desc
	*         Displays the message on LCD for configuration descriptor
	* @param  ConfDesc : Configuration descriptor
	* @retval None
	*/
	void configuration_desc_available(USBH_CfgDesc_TypeDef * cfgDesc,
											  USBH_InterfaceDesc_TypeDef *itfDesc,
											  USBH_EpDesc_TypeDef *epDesc)
	{
	  (void)epDesc;
	  (void)cfgDesc;
	  USBH_InterfaceDesc_TypeDef *id;

	  id = itfDesc;

	  if((*id).bInterfaceClass  == 0x08)
	  {
	   dbprintf("MSG_MSC_CLASS");
	  }
	  else if((*id).bInterfaceClass  == 0x03)
	  {
		  dbprintf("MSG_HID_CLASS");
	  }
	}

	void manufacturer_string(const char *ManufacturerString)
	{
	  dbprintf( "Manufacturer : %s\n", ManufacturerString);

	}

	void product_string(const char *ProductString)
	{
	  dbprintf( "Product : %s\n", ProductString);
	}

	void serialnum_string(const char *SerialNumString)
	{
		 dbprintf( "Serial Number : %s\n", SerialNumString);
	}

	void enumeration_done(void)
	{
		dbprintf( "Enumeration done");
	}


	USBH_USR_Status user_input(void)
	{

	  USBH_USR_Status usbh_usr_status;
	  dbprintf("User input");
	  usbh_usr_status = USBH_USR_RESP_OK;
	  return usbh_usr_status;
	}

	void device_not_supported(void)
	{
		dbprintf("> Device not supported.");

	}

	void unrecovered_error (void)
	{
		dbprintf("> Unrecovered error.");
	}

}}

static const USBH_Usr_cb_TypeDef usr_callbacks =
{
  usb_usr_cb::init,
  usb_usr_cb::deinit,
  usb_usr_cb::attached,
  usb_usr_cb::reset,
  usb_usr_cb::disconnected,
  usb_usr_cb::overcurrent,
  usb_usr_cb::speed_detected,
  usb_usr_cb::desc_available,
  usb_usr_cb::device_address_assigned,
  usb_usr_cb::configuration_desc_available,
  usb_usr_cb::manufacturer_string,
  usb_usr_cb::product_string,
  usb_usr_cb::serialnum_string,
  usb_usr_cb::enumeration_done,
  usb_usr_cb::user_input,
  NULL,
  usb_usr_cb::device_not_supported,
  usb_usr_cb::unrecovered_error
};
/* ------------------------------------------------------------------ */
/* USB int semaphore signal */
static isix::sem_t *usb_ready_sem;

namespace stm32 {
namespace dev {
/* ------------------------------------------------------------------ */
static void host_usb_task( void *entry_param )
{
	(void)entry_param;
	for(;;)
	{
		if( isix::isix_sem_wait( usb_ready_sem , isix::ISIX_TIME_INFINITE ) == isix::ISIX_EOK )
		{
			USBH_Process(&usb_otg_dev , &usb_host);
		}
	}
}
/* ------------------------------------------------------------------ */
/* Initialize USB bus */
int usb_bus_initialize( isix::dev::device &device )
{
	int ret = 0;
	::USBH_Init( &usb_otg_dev, USB_OTG_FS_CORE_ID, &usb_host, USBH_HID_Class_Callback(), &usr_callbacks );
	do
	{
		if( !(usb_ready_sem = isix::isix_sem_create_limited(NULL,0,1)) )
			break;
	}
	while(0);

	return ret;
}

/* ------------------------------------------------------------------ */
//OTG interrupt ISR vector
void __attribute__((__interrupt__)) otg_fs_isr_vector(void)
{
	const unsigned ret = USBH_OTG_ISR_Handler(&usb_otg_dev);
	if( ret )
	{
		isix::isix_sem_signal_isr( usb_ready_sem );
	}
}
/* ------------------------------------------------------------------ */
}}
