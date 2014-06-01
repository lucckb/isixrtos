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

//!Host operation type
enum usbh_async_type {
	USBH_ASYNC_OP = 0,			//! USBHost async operation
	USBH_SYNC_OP  = 1			//! USBHost sync operation
};

int usbh_control_request(int, usb_setup_packet_t const *, uint8_t *, uint32_t *);
int usbh_get_device(usb_speed_t *, uint8_t *, usb_device_descriptor_t *, unsigned );
usb_visible_state_t usbh_get_visible_device_state(void);
int usbh_set_class_machine(int (*)(void *), void (*)(void *, uint16_t),
                        void (*)(void *), void *);

#ifdef __cplusplus
}
#endif

#endif
