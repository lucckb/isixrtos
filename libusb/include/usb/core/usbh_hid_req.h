#ifndef _USBH_HID_REQ_H
#define _USBH_HID_REQ_H 1

#include <usb/core/usb_def.h>


#ifdef __cplusplus
extern "C" {
#endif

int usbh_hid_get_report_descriptor(int, uint8_t *, uint16_t);
int usbh_hid_set_idle(int, uint8_t, uint8_t, uint8_t);
int usbh_hid_set_boot_protocol(int, uint8_t, usb_hid_protocol_t);
int usbh_hid_set_report(int, uint8_t, uint8_t, uint8_t *, uint16_t);

#ifdef __cplusplus
}
#endif

#endif
