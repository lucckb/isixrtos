#ifndef _USBH_STD_REQ_H
#define _USBH_STD_REQ_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif



/** USB host standard request API **/

int usbh_set_device_address(int, uint8_t);
int usbh_set_configuration(int, uint8_t);
int usbh_clear_endpoint_halt(int, uint8_t);
int usbh_get_device_descriptor(int, uint8_t *, uint16_t);
int usbh_get_conf_descriptor(int, uint8_t, uint8_t *, uint16_t);
int usbh_get_string_descriptor_ascii(int, uint8_t, char *, unsigned *);

#ifdef __cplusplus
}
#endif

#endif
