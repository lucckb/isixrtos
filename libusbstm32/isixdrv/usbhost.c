/*
 * usbhost.c
 *
 *  Created on: 20-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include "usbh_hid_keybd.h"
#include <usbhost.h>
#include <usbh_core.h>
#include <usbh_hid_core.h>
#include <usb_hcd_int.h>
#include <dbglog.h>
#include <isix.h>

/* ------------------------------------------------------------------ */
//USB host core handle
static USB_OTG_CORE_HANDLE    usb_otg_dev;
static USBH_HOST usb_host;


void USBH_USR_Init(void)
{
	dbprintf("Init");
}

void USBH_USR_DeInit(void)
{
	dbprintf("Init");
}

void USBH_USR_DeviceAttached(void)
{
	dbprintf("Device attached");
}

void USBH_USR_ResetDevice(void)
{
	dbprintf("Device reset");
}


void USBH_USR_DeviceDisconnected (void)
{
	dbprintf("Device disconnected");
}

/**
* @brief  USBH_USR_OverCurrentDetected
*         Device Overcurrent detection event
* @param  None
* @retval None
*/
void USBH_USR_OverCurrentDetected (void)
{
	dbprintf("Overcurrent detected.\n");

}

void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
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

void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
  USBH_DevDesc_TypeDef *hs;
  hs = DeviceDesc;

 dbprintf( "VID : %04Xh\n" , (*hs).idVendor);
 dbprintf( "PID : %04Xh\n" , (*hs).idProduct);

}


void USBH_USR_DeviceAddressAssigned(void)
{
	dbprintf("Dev address assigned ");
}


#define KYBRD_FIRST_COLUMN               (uint16_t)319
#define KYBRD_LAST_COLUMN                (uint16_t)7
#define KYBRD_FIRST_LINE                 (uint8_t)120
#define KYBRD_LAST_LINE                  (uint8_t)200



/**
* @brief  USBH_USR_Conf_Desc
*         Displays the message on LCD for configuration descriptor
* @param  ConfDesc : Configuration descriptor
* @retval None
*/
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
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

void USBH_USR_Manufacturer_String(const char *ManufacturerString)
{
  dbprintf( "Manufacturer : %s\n", ManufacturerString);

}

void USBH_USR_Product_String(const char *ProductString)
{
  dbprintf( "Product : %s\n", ProductString);
}

void USBH_USR_SerialNum_String(const char *SerialNumString)
{
	 dbprintf( "Serial Number : %s\n", SerialNumString);
}

void USBH_USR_EnumerationDone(void)
{
	dbprintf( "Enumeration done");
}


USBH_USR_Status USBH_USR_UserInput(void)
{

  USBH_USR_Status usbh_usr_status;

  dbprintf("User input");
   usbh_usr_status = USBH_USR_RESP_OK;

  return usbh_usr_status;

}

void USBH_USR_DeviceNotSupported(void)
{
	dbprintf("> Device not supported.");

}

void USBH_USR_UnrecoveredError (void)
{
	dbprintf("> Unrecovered error.");
}



static const USBH_Usr_cb_TypeDef USR_Callbacks =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  NULL,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
};
/* ------------------------------------------------------------------ */
/* USB int semaphore signal */
static sem_t *usb_ready_sem;

/* ------------------------------------------------------------------ */
static ISIX_TASK_FUNC(host_usb_task, entry_param)
{
	(void)entry_param;
	for(;;)
	{
		if( isix_sem_wait( usb_ready_sem , ISIX_TIME_INFINITE ) == ISIX_EOK )
		{
			USBH_Process(&usb_otg_dev , &usb_host);
		}
	}
}
/* ------------------------------------------------------------------ */
int stm32_usbhost_init(void)
{
	isix_task_create( host_usb_task, NULL, 1024, 3 );
	int ret = 0;
	USBH_Init( &usb_otg_dev, USB_OTG_FS_CORE_ID, &usb_host, USBH_HID_Class_Callback(), &USR_Callbacks );
	do
	{
		if( !(usb_ready_sem = isix_sem_create_limited(NULL,0,1)) )
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
		isix_sem_signal_isr( usb_ready_sem );
	}
}
/* ------------------------------------------------------------------ */
