#ifndef _USBH_STD_REQ_H
#define _USBH_STD_REQ_H 1

#include <usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/** USB host standard request API **/

int USBHsetDeviceAddress(int, uint8_t);
int USBHsetConfiguration(int, uint8_t);
int USBHclearEndpointHalt(int, uint8_t);
int USBHgetDeviceDescriptor(int, uint8_t *, uint16_t);
int USBHgetConfDescriptor(int, uint8_t, uint8_t *, uint16_t);
int USBHgetStringDescriptorASCII(int, uint8_t, char *, unsigned *);

#ifdef __cplusplus
}
#endif

#endif
