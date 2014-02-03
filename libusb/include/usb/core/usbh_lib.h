#ifndef _USBH_LIB_H
#define _USBH_LIB_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

int usbh_open_device(usb_speed_t *, uint8_t *,
                   usb_device_descriptor_t *, unsigned);
int usbh_is_device_ready(void);
void usbh_device_hard_reset(unsigned);

#ifdef __cplusplus
}
#endif

#endif
