#ifndef _USBH_LIB_H
#define _USBH_LIB_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

int USBHopenDevice(usb_speed_t *, uint8_t *,
                   usb_device_descriptor_t *, unsigned);
int USBHisDeviceReady(void);
void USBHdeviceHardReset(unsigned);

#ifdef __cplusplus
}
#endif

#endif
