/*
 * usbhost.c
 *
 *  Created on: 20-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include "usbh_hid_mouse.h"
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


 dbprintf( "VID : %04Xh\n" , (uint32_t)(*hs).idVendor);


 dbprintf( "PID : %04Xh\n" , (uint32_t)(*hs).idProduct);
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
* @}
*/
uint8_t  KeybrdCharXpos           = 0;
uint16_t KeybrdCharYpos           = 0;
extern  int16_t  x_loc, y_loc;
extern __IO int16_t  prev_x, prev_y;

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

void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
  dbprintf( "Manufacturer : %s\n", (char *)ManufacturerString);

}

void USBH_USR_Product_String(void *ProductString)
{
  dbprintf( "Product : %s\n", (char *)ProductString);
}

void USBH_USR_SerialNum_String(void *SerialNumString)
{
	 dbprintf( "Serial Number : %s\n", (char *)SerialNumString);
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

void  USR_KEYBRD_Init (void)
{
	dbprintf("> KEYB INIT.");
}

void  USR_KEYBRD_ProcessData (uint8_t data)
{


    dbprintf("KBD event [%c]", data);

}

void USR_MOUSE_Init	(void)
{
	dbprintf("Mouse initialized");
}


void USR_MOUSE_ProcessData(HID_MOUSE_Data_TypeDef *data)
{

  uint8_t idx = 1;
  static uint8_t b_state[3] = { 0, 0 , 0};

  if ((data->x != 0) && (data->y != 0))
  {
    HID_MOUSE_UpdatePosition(data->x , data->y);
  }

  for ( idx = 0 ; idx < 3 ; idx ++)
  {

    if(data->button & 1 << idx)
    {
      if(b_state[idx] == 0)
      {
        HID_MOUSE_ButtonPressed (idx);
        b_state[idx] = 1;
      }
    }
    else
    {
      if(b_state[idx] == 1)
      {
        HID_MOUSE_ButtonReleased (idx);
        b_state[idx] = 0;
      }
    }
  }


}

void HID_MOUSE_UpdatePosition (int8_t x, int8_t y)
{
	dbprintf("MOuse update pos %i %i", x, y);

}
void HID_MOUSE_ButtonPressed(uint8_t button_idx)
{
	dbprintf("Mouse pressed %u", button_idx);
}

void HID_MOUSE_ButtonReleased(uint8_t button_idx)
{
	dbprintf("Mouse released %u", button_idx);
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
static bool reset_req = false;
/* ------------------------------------------------------------------ */
static ISIX_TASK_FUNC(host_usb_task, entry_param)
{
	(void)entry_param;
	for(;;)
	{
		if( isix_sem_wait( usb_ready_sem , ISIX_TIME_INFINITE ) == ISIX_EOK )
		{
			if( reset_req )
			{
				USB_OTG_ResetPort(&usb_otg_dev);
				dbprintf("Reset port req");
				reset_req = false;
			}
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
		if( ret & usbh_hcd_do_reset_required )
		{
			reset_req = true;
		}
	}
}
/* ------------------------------------------------------------------ */
