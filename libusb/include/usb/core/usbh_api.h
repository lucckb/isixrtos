#ifndef _USBH_API_H
#define _USBH_API_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/** USB host low level API **/

unsigned USBHgetChannelCount(void);
void USBHstopAllChannels(void);
void USBHinitChannel(int, uint8_t, uint8_t,
                     usb_speed_t, usb_transfer_t, uint16_t);
int USBHprepareChannel(int, uint32_t, usb_pid_t);
void USBHhaltChannel(int);

uint16_t USBHgetCurrentFrame(void);
uint16_t USBHgetFrameClocksRemaining(void);

#ifdef __cplusplus
}
#endif

#endif
