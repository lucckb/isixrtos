/*
 * usbserial.c
 *
 *  Created on: 10-11-2012
 *      Author: lucck
 */

/* ------------------------------------------------------------------ */
#include "usbdevserial.h"
#include <usbd_cdc_core.h>
#include <usbd_req.h>
#include <usb_dcd_int.h>
#include <dbglog.h>
#include <isix.h>
/* ------------------------------------------------------------------ */

#define USBD_VID                        0x0483
#define USBD_PID                        0x5740
#define USB_SIZ_STRING_LANGID            4
#define USBD_LANGID_STRING              0x409
#define USBD_MANUFACTURER_STRING        "STMicroelectronics"
#define USBD_PRODUCT_HS_STRING          "STM32 Virtual ComPort in HS mode"
#define USBD_SERIALNUMBER_HS_STRING     "00000000050B"
#define USBD_PRODUCT_FS_STRING          "STM32 Virtual ComPort in FS Mode"
#define USBD_SERIALNUMBER_FS_STRING     "00000000050C"
#define USBD_CONFIGURATION_HS_STRING    "VCP Config"
#define USBD_INTERFACE_HS_STRING        "VCP Interface"
#define USBD_CONFIGURATION_FS_STRING    "VCP Config"
#define USBD_INTERFACE_FS_STRING        "VCP Interface"

/* ------------------------------------------------------------------ */
//Callbacks get descs
static const uint8_t* get_device_descriptor( uint8_t speed , uint16_t *length );
static const uint8_t* get_langid_str_descriptor( uint8_t speed , uint16_t *length );
static const uint8_t* get_manufacturer_str_descriptor( uint8_t speed , uint16_t *length );
static const uint8_t* get_product_str_descriptor( uint8_t speed , uint16_t *length );
static const uint8_t* get_serial_str_descriptor( uint8_t speed , uint16_t *length );
static const uint8_t* get_configuration_str_descriptor( uint8_t speed , uint16_t *length );
static const uint8_t* get_interface_str_descriptor( uint8_t speed , uint16_t *length );
/* ------------------------------------------------------------------ */
// Device callbacks family
static void device_init_cb(void);
static void device_reset_cb(uint8_t speed);
static void device_configured_cb(void);
static void device_suspended_cb(void);
static void device_resumed_cb(void);
static void device_connected_cb(void);
static void device_disconnected_cb(void);
/* ------------------------------------------------------------------ */
//CDC class callbacks
static int cdc_init_cb(void);
static int cdc_deinit_cb(void);
static int cdc_control_cb(uint32_t cmd, uint8_t* buf, uint32_t len);
static int cdc_data_tx (const uint8_t** buf, uint32_t len);
static int cdc_data_rx (const uint8_t* buf, uint32_t len);
/* ------------------------------------------------------------------ */
//USB dev core handle
static USB_OTG_CORE_HANDLE    usb_otg_dev;

/* ------------------------------------------------------------------ */
//USB Device descriptor structure
static const USBD_DEVICE usr_desc =
{
	get_device_descriptor,
	get_langid_str_descriptor,
	get_manufacturer_str_descriptor,
	get_product_str_descriptor,
	get_serial_str_descriptor,
	get_configuration_str_descriptor,
	get_interface_str_descriptor
};
/* ------------------------------------------------------------------ */
//User device callbacks
static const USBD_Usr_cb_TypeDef usr_cb =
{
	device_init_cb,
	device_reset_cb,
	device_configured_cb,
	device_suspended_cb,
	device_resumed_cb,
	device_connected_cb,
	device_disconnected_cb
};

/* ------------------------------------------------------------------ */
// CDC specific class operation
static const CDC_IF_Prop_TypeDef cdc_if_ops =
{
	cdc_init_cb,
	cdc_deinit_cb,
	cdc_control_cb,
	cdc_data_tx,
	cdc_data_rx
};

/* ------------------------------------------------------------------ */
//Str desc buffer
static uint8_t str_desc[USB_MAX_STR_DESC_SIZ];

/* ------------------------------------------------------------------ */
static const uint8_t* get_device_descriptor( uint8_t speed , uint16_t *length )
{
	(void)speed;
	__ALIGN_BEGIN static const uint8_t dev_desc[] __ALIGN_END =
	  {
	    0x12,                       /*bLength */
	    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
	    0x00,                       /*bcdUSB */
	    0x02,
	    DEVICE_CLASS_CDC,           /*bDeviceClass*/
	    DEVICE_SUBCLASS_CDC,        /*bDeviceSubClass*/
	    0x00,                       /*bDeviceProtocol*/
	    USB_OTG_MAX_EP0_SIZE,      /*bMaxPacketSize*/
	    LOBYTE(USBD_VID),           /*idVendor*/
	    HIBYTE(USBD_VID),           /*idVendor*/
	    LOBYTE(USBD_PID),           /*idVendor*/
	    HIBYTE(USBD_PID),           /*idVendor*/
	    0x00,                       /*bcdDevice rel. 2.00*/
	    0x02,
	    USBD_IDX_MFC_STR,           /*Index of manufacturer  string*/
	    USBD_IDX_PRODUCT_STR,       /*Index of product string*/
	    USBD_IDX_SERIAL_STR,        /*Index of serial number string*/
	    USBD_CFG_MAX_NUM            /*bNumConfigurations*/
	  } ; /* USB_DeviceDescriptor */
	*length = sizeof(dev_desc);
	return dev_desc;
}
/* ------------------------------------------------------------------ */
static const uint8_t* get_langid_str_descriptor( uint8_t speed , uint16_t *length )
{
	(void)speed;
	__ALIGN_BEGIN static const uint8_t lang_id_desc[] __ALIGN_END =
	{
	     USB_SIZ_STRING_LANGID,
	     USB_DESC_TYPE_STRING,
	     LOBYTE(USBD_LANGID_STRING),
	     HIBYTE(USBD_LANGID_STRING),
	};
	*length = sizeof(lang_id_desc);
	return lang_id_desc;
}

/* ------------------------------------------------------------------ */
static const uint8_t* get_manufacturer_str_descriptor( uint8_t speed , uint16_t *length )
{
	(void)speed;
	USBD_GetString(USBD_MANUFACTURER_STRING, str_desc, length);
	return str_desc;
}
/* ------------------------------------------------------------------ */
static const uint8_t* get_product_str_descriptor( uint8_t speed , uint16_t *length )
{
	if( speed == 0 )
	{
		USBD_GetString (USBD_PRODUCT_HS_STRING, str_desc, length);
		//*length = sizeof(USBD_PRODUCT_HS_STRING)/sizeof(USBD_PRODUCT_HS_STRING[0]);
		//return (const uint8_t*)USBD_PRODUCT_HS_STRING;
	}
	else
	{
		USBD_GetString (USBD_PRODUCT_FS_STRING, str_desc, length);
		//*length = sizeof(USBD_PRODUCT_FS_STRING)/sizeof(USBD_PRODUCT_FS_STRING[0]);
		//return (const uint8_t*)USBD_PRODUCT_FS_STRING;
	}
	return str_desc;
}
/* ------------------------------------------------------------------ */
static const uint8_t* get_serial_str_descriptor( uint8_t speed , uint16_t *length )
{
	if(speed  == USB_OTG_SPEED_HIGH)
	{
	    USBD_GetString (USBD_SERIALNUMBER_HS_STRING, str_desc, length);
	}
	else
	{
	   USBD_GetString (USBD_SERIALNUMBER_FS_STRING, str_desc, length);
	}
	return str_desc;
}
/* ------------------------------------------------------------------ */
static const uint8_t* get_configuration_str_descriptor( uint8_t speed , uint16_t *length )
{
	if(speed  == USB_OTG_SPEED_HIGH)
	{
	   USBD_GetString (USBD_CONFIGURATION_HS_STRING, str_desc, length);
	}
	else
	{
	   USBD_GetString (USBD_CONFIGURATION_FS_STRING, str_desc, length);
	}
	return str_desc;
}
/* ------------------------------------------------------------------ */
static const uint8_t* get_interface_str_descriptor( uint8_t speed , uint16_t *length )
{
	if(speed == 0)
	{
	   USBD_GetString (USBD_INTERFACE_HS_STRING, str_desc, length);
	}
	else
	{
	    USBD_GetString (USBD_INTERFACE_FS_STRING, str_desc, length);
	}
	return str_desc;
}
/* ------------------------------------------------------------------ */
// Device callbacks family
static void device_init_cb(void)
{
	dbprintf("Device initialized");
}
/* ------------------------------------------------------------------ */
static void device_reset_cb(uint8_t speed)
{
	dbprintf("device reset speed=%d", speed);
}
/* ------------------------------------------------------------------ */
static void device_configured_cb(void)
{
	dbprintf("Device configured");
}
/* ------------------------------------------------------------------ */
static void device_suspended_cb(void)
{
	dbprintf("Device supspedned");
}
/* ------------------------------------------------------------------ */
static void device_resumed_cb(void)
{
	dbprintf("device resumed");
}
/* ------------------------------------------------------------------ */
static void device_connected_cb(void)
{
	dbprintf("Device connected");
}
/* ------------------------------------------------------------------ */
static void device_disconnected_cb(void)
{
	dbprintf("Device disconnected");
}
/* ------------------------------------------------------------------ */
static int cdc_init_cb(void)
{
	dbprintf("cdc init");
	return USBD_OK;
}
/* ------------------------------------------------------------------ */
static int cdc_deinit_cb(void)
{
	dbprintf("cdc deinit");
	return USBD_OK;
}
/* ------------------------------------------------------------------ */
static int cdc_control_cb(uint32_t cmd, uint8_t* buf, uint32_t len)
{
	(void)buf;
	dbprintf("cdc control %d l=%d", cmd, len);
	return USBD_OK;
}
/* ------------------------------------------------------------------ */
static int cdc_data_tx (const uint8_t** buf, uint32_t len)
{
	*buf = NULL;
	//dbprintf("cdc tx = %d", len );
	return USBD_OK;
}
/* ------------------------------------------------------------------ */
static int cdc_data_rx (const uint8_t* buf, uint32_t len)
{
	(void)buf;
	dbprintf("cdc rx = %d", len );
	return USBD_OK;
}

/* ------------------------------------------------------------------ */
/* Initialize the USB serial module */
int stm32_usbdev_serial_init( size_t rx_fifo_len, size_t tx_fifo_len  )
{
	(void)rx_fifo_len;
	(void)tx_fifo_len;
	USBD_Init( &usb_otg_dev, USB_OTG_FS_CORE_ID, &usr_desc,
			cdc_class_init(&cdc_if_ops), &usr_cb);
	return 0;
}
/* ------------------------------------------------------------------ */
//OTG interrupt ISR vector
void __attribute__((__interrupt__)) otg_fs_isr_vector(void)
{
	USBD_OTG_ISR_Handler(&usb_otg_dev);
}
/* ------------------------------------------------------------------ */
