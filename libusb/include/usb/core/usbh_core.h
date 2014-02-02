#ifndef _USBH_CORE_H
#define _USBH_CORE_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/** USB host core system API **/

int  USBHcoreConfigure(void);
void USBHdeviceDisconnected(void);
void USBHdeviceAttached(void);
void USBHdeviceSpeed(usb_speed_t);
void USBHdeviceResetDone(void*);
void USBHsof(uint16_t);
void USBHcoreProcess(void);

/** USB host user API **/

int USBHcontrolRequest(int, usb_setup_packet_t const *, uint8_t *, uint32_t *);
int USBHgetDevice(usb_speed_t *, uint8_t *, usb_device_descriptor_t *);
usb_visible_state_t USBHgetVisibleDeviceState(void);
int USBHsetClassMachine(int (*)(void *), void (*)(void *, uint16_t),
                        void (*)(void *), void *);

#ifdef __cplusplus
}
#endif

#endif
