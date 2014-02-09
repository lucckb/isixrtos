#ifndef _USBH_HID_CORE_H
#define _USBH_HID_CORE_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int new_mouse_data;
extern unsigned mouse_buttons;
extern int mouse_x, mouse_y;

extern int new_keyboard_data;
extern unsigned keyboard_modifiers;
extern uint8_t keyboard_scan_code[KEYBOARD_MAX_PRESSED_KEYS];

struct usbh_hid_context;
typedef struct usbh_hid_context usbh_hid_context_t;

usbh_hid_context_t* usbh_hid_core_new_ctx(void);

int usbh_hid_set_machine( usbh_hid_context_t *hid_ctx,
				  usb_speed_t speed, uint8_t dev_addr,
                  usb_interface_descriptor_t const *if_desc,
                  usb_hid_main_descriptor_t const *hid_desc,
                  usb_endpoint_descriptor_t const *ep_desc,
                  unsigned ep_count);

int usbh_hid_is_device_ready(const usbh_hid_context_t* hid_ctx);

int usbh_hid_error(const usbh_hid_context_t* hid_ctx);

#ifdef __cplusplus
}
#endif

#endif
