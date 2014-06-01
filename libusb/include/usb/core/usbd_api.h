#ifndef _USBD_API_H
#define _USBD_API_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif
/** General API **/

typedef enum {
  SET_ADDRESS_RESET, SET_ADDRESS_REQUEST, SET_ADDRESS_STATUS
} set_address_t;

void USBDsetDeviceAddress(set_address_t, uint8_t);

/** Library private API **/

void USBDdataReceived(uint8_t, uint32_t);
void USBDcontinueInTransfer(uint8_t);

/** Endpoint configuration API **/

void         USBDdisableAllNonControlEndPoints(void);
usb_result_t USBDendPointConfigure(uint8_t, usb_transfer_t,
                                   uint16_t, uint16_t);
usb_result_t USBDendPointConfigureEx(uint8_t, usb_transfer_t,
                                     uint16_t, uint16_t, uint16_t);

/** Endpoint API **/

uint16_t USBDwrite(uint8_t, uint8_t const *, uint16_t);
uint32_t USBDwriteEx(uint8_t, uint8_t const *, uint32_t);
uint16_t USBDread(uint8_t, uint8_t *, uint16_t);
usb_result_t USBDgetEndPointStatus(uint8_t, uint16_t *);
usb_result_t USBDsetEndPointHalt(uint8_t);
usb_result_t USBDclearEndPointHalt(uint8_t);

/** Default control endpoint shortcuts **/

uint16_t USBDwrite0(uint8_t const *, uint16_t);
uint16_t USBDread0(uint8_t *, uint16_t);
void     USBDendPoint0TxVALID(void);
void     USBDendPoint0RxVALID(void);
void     USBDendPoint0TxSTALL(void);
void     USBDendPoint0RxSTALL(void);

#ifdef __cplusplus
}
#endif

#endif
