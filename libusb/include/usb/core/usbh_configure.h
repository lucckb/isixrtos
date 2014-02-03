#ifndef _USBH_CONFIGURE_H
#define _USBH_CONFIGURE_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/** USB host initialization API **/

int usbh_configure(usb_phy_t);
void USBHvbus(int);

#ifdef __cplusplus
}
#endif

#endif
