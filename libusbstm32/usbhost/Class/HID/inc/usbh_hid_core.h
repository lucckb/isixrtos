/**
  ******************************************************************************
  * @file    usbh_hid_core.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file contains all the prototypes for the usbh_hid_core.c
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

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef _USBH_HID_CORE_H
#define _USBH_HID_CORE_H

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"
#include "usbh_stdreq.h"
#include "usb_bsp.h"
#include "usbh_ioreq.h"
#include "usbh_hcs.h"
#include <memory>
#include <usb_device.hpp>
#include <gfx/input/input.hpp>

namespace isix {
namespace dev {
	class usb_host;
}
}



static constexpr auto HID_MIN_POLL    =      10;



/* States for HID State Machine */
typedef enum
{
  HID_IDLE= 0,
  HID_SEND_DATA,
  HID_BUSY,
  HID_GET_DATA,   
  HID_SYNC,     
  HID_POLL,
  HID_ERROR
}
HID_State;

typedef enum
{
  HID_REQ_IDLE = 0,
  HID_REQ_GET_REPORT_DESC,
  HID_REQ_GET_HID_DESC,
  HID_REQ_SET_IDLE,
  HID_REQ_SET_PROTOCOL,
  HID_REQ_SET_REPORT

}
HID_CtlState;


typedef  struct  _HID_Report 
{
    uint8_t   ReportID;    
    uint8_t   ReportType;  
    uint16_t  UsagePage;   
    uint32_t  Usage[2]; 
    uint32_t  NbrUsage;                      
    uint32_t  UsageMin;                      
    uint32_t  UsageMax;                      
    int32_t   LogMin;                        
    int32_t   LogMax;                        
    int32_t   PhyMin;                        
    int32_t   PhyMax;                        
    int32_t   UnitExp;                       
    uint32_t  Unit;                          
    uint32_t  ReportSize;                    
    uint32_t  ReportCnt;                     
    uint32_t  Flag;                          
    uint32_t  PhyUsage;                      
    uint32_t  AppUsage;                      
    uint32_t  LogUsage;   
} 
HID_Report_TypeDef;

class usb_input_device	: public gfx::inp::input_class, public isix::dev::usb_device
{
public:
	usb_input_device( isix::dev::usb_host & host, isix::dev::device::class_id id )
		: input_class( id ), usb_device( host )
	{}
};

/* Structure for HID process */
typedef struct _HID_Process
{
  uint8_t              buff[64];
  uint8_t              hc_num_in; 
  uint8_t              hc_num_out; 
  HID_State            state; 
  uint8_t              HIDIntOutEp;
  uint8_t              HIDIntInEp;
  HID_CtlState         ctl_state;
  uint16_t             length;
  uint8_t              ep_addr;
  uint16_t             poll; 
  __IO uint16_t        timer; 
  std::shared_ptr<usb_input_device> cb;
}
HID_Machine_TypeDef;





/** @defgroup USBH_HID_CORE_Exported_Defines
  * @{
  */ 

#define USB_HID_REQ_GET_REPORT       0x01
#define USB_HID_GET_IDLE             0x02
#define USB_HID_GET_PROTOCOL         0x03
#define USB_HID_SET_REPORT           0x09
#define USB_HID_SET_IDLE             0x0A
#define USB_HID_SET_PROTOCOL         0x0B    


#ifdef __cplusplus
extern "C" {
#endif

const USBH_Class_cb_TypeDef * USBH_HID_Class_Callback( isix::dev::usb_host *host );


std::shared_ptr<usb_input_device> USBH_HID_Get_Object();


/** @defgroup USBH_HID_CORE_Exported_FunctionsPrototype
  * @{
  */ 

USBH_Status USBH_Set_Report (USB_OTG_CORE_HANDLE *pdev,
                             USBH_HOST *phost,
                                  uint8_t reportType,
                                  uint8_t reportId,
                                  uint8_t reportLen,
                                  uint8_t* reportBuff);



#ifdef __cplusplus
}
#endif


#endif /* __USBH_HID_CORE_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

