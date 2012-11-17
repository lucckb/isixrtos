/**
  ******************************************************************************
  * @file    usbd_cdc_core.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   header file for the usbd_cdc_core.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#ifndef __USB_CDC_CORE_H_
#define __USB_CDC_CORE_H_

#include  "usbd_ioreq.h"

//#ifndef APP_RX_DATA_SIZE
//#define APP_RX_DATA_SIZE               1024
//#endif

#ifndef CDC_IN_EP
#define CDC_IN_EP                       0x81  /* EP1 for data IN */
#endif

#ifndef CDC_OUT_EP
#define CDC_OUT_EP                      0x01  /* EP1 for data OUT */
#endif

#ifndef CDC_CMD_EP
#define CDC_CMD_EP                      0x82  /* EP2 for CDC commands */
#endif

#ifndef CDC_CMD_PACKET_SZE
#define CDC_CMD_PACKET_SZE             8    /* Control Endpoint Packet size */
#endif

#ifndef CDC_DATA_MAX_PACKET_SIZE
#ifdef USE_USB_OTG_HS
#define CDC_DATA_MAX_PACKET_SIZE       512
#else
#define CDC_DATA_MAX_PACKET_SIZE       64   /* Endpoint IN & OUT Packet size */
#endif
#endif

#ifndef CDC_IN_FRAME_INTERVAL
#define CDC_IN_FRAME_INTERVAL          5    /* Number of frames between IN transfers */
#endif

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup usbd_cdc
  * @brief This file is the Header file for USBD_cdc.c
  * @{
  */ 


/** @defgroup usbd_cdc_Exported_Defines
  * @{
  */ 
#define USB_CDC_CONFIG_DESC_SIZ                (67)
#define USB_CDC_DESC_SIZ                       (67-9)

#define CDC_DESCRIPTOR_TYPE                     0x21

#define DEVICE_CLASS_CDC                        0x02
#define DEVICE_SUBCLASS_CDC                     0x00


#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define CDC_DATA_IN_PACKET_SIZE                CDC_DATA_MAX_PACKET_SIZE
        
#define CDC_DATA_OUT_PACKET_SIZE               CDC_DATA_MAX_PACKET_SIZE

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/

/**************************************************/
/* CDC Requests                                   */
/**************************************************/
#define SEND_ENCAPSULATED_COMMAND               0x00
#define GET_ENCAPSULATED_RESPONSE               0x01
#define SET_COMM_FEATURE                        0x02
#define GET_COMM_FEATURE                        0x03
#define CLEAR_COMM_FEATURE                      0x04
#define SET_LINE_CODING                         0x20
#define GET_LINE_CODING                         0x21
#define SET_CONTROL_LINE_STATE                  0x22
#define SEND_BREAK                              0x23
#define NO_CMD                                  0xFF


/*---------------------------------------------------------------------*/
/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
typedef struct _CDC_IF_PROP
{
  int (*pIf_Init)     (void);
  int (*pIf_DeInit)   (void);
  int (*pIf_Ctrl)     (uint32_t cmd, uint8_t* buf, uint32_t len);
  int (*pIf_DataTx)   (const uint8_t** buf);
  int (*pIf_DataRx)   (const uint8_t* buf, uint32_t len);
}
CDC_IF_Prop_TypeDef;

/*---------------------------------------------------------------------*/

const USBD_Class_cb_TypeDef* cdc_class_init( const CDC_IF_Prop_TypeDef  *app_fops );

/*---------------------------------------------------------------------*/

#endif  // __USB_CDC_CORE_H_
