#ifndef _USBD_CORE_H
#define _USBD_CORE_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/** USB device core system API **/

int USBDcoreConfigure(void);
void USBDreset(usb_speed_t);
void USBDsuspend(void);
void USBDwakeup(void);
void USBDsof(uint16_t);
void USBDtransfer(uint8_t, usb_pid_t);

#ifdef __cplusplus

}
#endif
#endif
