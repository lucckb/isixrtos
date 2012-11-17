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
#include <prv/list.h>
#include <string.h>
/* ------------------------------------------------------------------ */
#define USB_PACKET_TX_BUF_NBUFS			4
#define USB_PACKET_RX_BUF_NBUFS			8
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
#if USB_PACKET_TX_BUF_NBUFS < 4
#error Minimum 4 TX packet buffers are required
#endif
#if USB_PACKET_RX_BUF_NBUFS < 4
#error Minimum 4 RX packet buffers are required
#endif
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
static int cdc_data_tx (const uint8_t** buf);
static void* cdc_data_rx (const void* buf, uint32_t len);
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
static uint8_t str_desc[USB_MAX_STR_DESC_SIZ*2];

/* ------------------------------------------------------------------ */
//USB packet buffer
struct usbpkt_buf
{
	uint8_t usb_pkt[CDC_DATA_MAX_PACKET_SIZE];
	uint8_t pkt_len;
};
typedef struct usbpkt_buf usbpkt_buf_t;
/* ------------------------------------------------------------------ */
//TX fifo packet hdr
static fifo_t* tx_fifo;
//TX mempool
static isix_mempool_t tx_mempool;
//RX fifo packet hdr
static fifo_t* rx_fifo;
//RX mempool
static isix_mempool_t rx_mempool;
//API lock sem
static sem_t * lock_sem;
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
	(void)cmd;
	(void)len;
	//dbprintf("cdc control %d l=%d", cmd, len);
	return USBD_OK;
}
/* ------------------------------------------------------------------ */
//Transmit data called from USB irq context
static int cdc_data_tx (const uint8_t** buf)
{
	static usbpkt_buf_t *proc_pkt;
	if( proc_pkt )
	{
		isix_mempool_free( tx_mempool, proc_pkt );
		proc_pkt = NULL;
	}
	if( isix_fifo_read_isr(tx_fifo, &proc_pkt ) != ISIX_EOK )
	{
		*buf = NULL;
		return 0;
	}
	*buf = proc_pkt->usb_pkt;
	return proc_pkt->pkt_len;
}
//__attribute__((optimize("-O3")))
/* ------------------------------------------------------------------ */
/* Write data to the virtual serial com port
 * @param[in] buf Pointer to data buffer
 * @param[in] buf_len Buffer length
 * @param[in] timeout Timeout or ISIX_TIME_INFINITE
 * @return Number of bytes written or negative error code if fail
 */
int stm32_usbdev_serial_write( const void *buf, size_t buf_len, unsigned timeout )
{
	if( usb_otg_dev.dev.device_status != USB_OTG_CONFIGURED )
		return STM32_USBDEV_ERR_NOT_CONNECTED;
	int is = isix_sem_wait( lock_sem, ISIX_TIME_INFINITE );
	if( is != ISIX_EOK )
		return is;
	int ret = 0;
	while( buf_len > 0 )
	{
		const uint8_t *_buf = buf;
		usbpkt_buf_t* p = isix_mempool_alloc(tx_mempool);
		if( !p ) break;
		const int pwr = buf_len>sizeof(p->usb_pkt)?sizeof(p->usb_pkt):buf_len;
		memcpy( &p->usb_pkt, &_buf[ret], pwr );
		p->pkt_len = pwr;
		is = isix_fifo_write( tx_fifo, &p, timeout );
		if( is != ISIX_EOK )
		{
			isix_mempool_free( tx_mempool, p );
			if (is!=ISIX_ETIMEOUT) ret = is;
			break;
		}
		buf_len -= pwr;
		ret += pwr;
	}
	if( (is = isix_sem_signal( lock_sem )) != ISIX_EOK )
		return is;
	else return ret;
}
/* ------------------------------------------------------------------ */
//Receive data called from USB irq context
static void* cdc_data_rx (const void* buf, uint32_t len)
{
	if( len > 0 )
	{
		//Update pkt len
		((usbpkt_buf_t*)buf)->pkt_len = len;
		if( isix_fifo_write_isr( rx_fifo, &buf ) != ISIX_EOK )
		{
			return NULL;
		}
	}
	return isix_mempool_alloc( rx_mempool );
}
/* ------------------------------------------------------------------ */
/* read data from the virtual serial com port
 * @param[out] buf Pointer to data buffer
 * @param[in] buf_len Buffer length
 * @param[in] timeout Timeout or ISIX_TIME_INFINITE
 * @return Number of bytes read or negative error code if fail
 */
int stm32_usbdev_serial_read( void *buf, const size_t buf_len, int tout_mode )
{
	if( usb_otg_dev.dev.device_status != USB_OTG_CONFIGURED )
		return STM32_USBDEV_ERR_NOT_CONNECTED;
	int is = isix_sem_wait( lock_sem, ISIX_TIME_INFINITE );
	if( is != ISIX_EOK )
		return is;
	int rd = 0;
	uint8_t *_buf = buf;
	while( rd < (int)buf_len )
	{
		usbpkt_buf_t* p = NULL;
		if( tout_mode==USBDEV_SERIAL_NONBLOCK )
		{
			if( isix_fifo_count(rx_fifo) == 0 )
				break;
		}
		int is = isix_fifo_read( rx_fifo, &p, tout_mode>0?tout_mode:ISIX_TIME_INFINITE );
		if( is != ISIX_EOK )
		{
			if( is != ISIX_ETIMEOUT ) rd = is;
			break;
		}
		memcpy( &_buf[rd], p->usb_pkt, p->pkt_len<=buf_len?p->pkt_len:buf_len );
		rd += p->pkt_len;
		isix_mempool_free( rx_mempool, p );
		if(tout_mode == USBDEV_SERIAL_BLOCK_TO_DATA_AVAIL || tout_mode > 0)
		{
			//Break when no data avail
			if( rd > 0 && isix_fifo_count(rx_fifo)==0 )
				break;
		}
	}
	if( (is = isix_sem_signal( lock_sem )) != ISIX_EOK )
		return is;
	else return rd;
}
/* ------------------------------------------------------------------ */
/* Initialize the USB serial module */
int stm32_usbdev_serial_open( void )
{
	if( lock_sem )
		return ISIX_EINVARG;
	int ret = ISIX_EOK;
	do
	{
		if( !(lock_sem = isix_sem_create_limited(NULL,1,1)) )
			{ ret=ISIX_ENOMEM; break; }
		if(!(tx_mempool = isix_mempool_create(USB_PACKET_TX_BUF_NBUFS,sizeof(usbpkt_buf_t))) )
			{ ret=ISIX_ENOMEM; break; }
		if( !(tx_fifo = isix_fifo_create(USB_PACKET_TX_BUF_NBUFS - 2, sizeof(usbpkt_buf_t*))) )
			{ ret=ISIX_ENOMEM; break; }
		if( !(rx_mempool = isix_mempool_create( USB_PACKET_RX_BUF_NBUFS, sizeof(usbpkt_buf_t) )) )
			{ ret=ISIX_ENOMEM; break; }
		if( !(rx_fifo = isix_fifo_create(USB_PACKET_RX_BUF_NBUFS - 2, sizeof(usbpkt_buf_t*))) )
			{ ret=ISIX_ENOMEM; break; }
	} while(0);
	if( ret != ISIX_EOK )
	{
		if( lock_sem ) 	 isix_sem_destroy( lock_sem );
		if( rx_mempool ) isix_mempool_destroy( rx_mempool );
		if( tx_mempool ) isix_mempool_destroy( tx_mempool );
		if( rx_fifo )    isix_fifo_destroy( rx_fifo );
		if( tx_fifo )    isix_fifo_destroy( tx_fifo );
		lock_sem = NULL;
	}
	else
	{
		USBD_Init( &usb_otg_dev, USB_OTG_FS_CORE_ID, &usr_desc,
				cdc_class_init(&cdc_if_ops), &usr_cb);
	}
	return ret;
}
/* ------------------------------------------------------------------ */
/**  Close the USB serial module
 */
void stm32_usbdev_serial_close(void)
{
	if( !lock_sem ) return;
	isix_sem_wait( lock_sem, ISIX_TIME_INFINITE );
	/* Deinitialize the USB stuff */
	USBD_DeInit( &usb_otg_dev );
	isix_sem_destroy( lock_sem );
	if( rx_mempool ) isix_mempool_destroy( rx_mempool );
	if( tx_mempool ) isix_mempool_destroy( tx_mempool );
	if( rx_fifo ) isix_fifo_destroy( rx_fifo );
	if( tx_fifo ) isix_fifo_destroy( tx_fifo );
}
/* ------------------------------------------------------------------ */
//OTG interrupt ISR vector
void __attribute__((__interrupt__)) otg_fs_isr_vector(void)
{
	USBD_OTG_ISR_Handler(&usb_otg_dev);
}
/* ------------------------------------------------------------------ */
