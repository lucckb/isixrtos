#ifndef _USBH_HID_REQ_H
#define _USBH_HID_REQ_H 1

#include <usb/core/usb_def.h>


#ifdef __cplusplus
extern "C" {
#endif

int HIDgetReportDescriptor(int, uint8_t *, uint16_t);
int HIDsetIdle(int, uint8_t, uint8_t, uint8_t);
int HIDsetBootProtocol(int, uint8_t, usb_hid_protocol_t);
int HIDsetReport(int, uint8_t, uint8_t, uint8_t *, uint16_t);

#ifdef __cplusplus
}
#endif

#endif
