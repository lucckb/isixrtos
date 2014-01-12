#ifndef _USBH_HID_CORE_H
#define _USBH_HID_CORE_H 1

#include <usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int new_mouse_data;
extern unsigned mouse_buttons;
extern int mouse_x, mouse_y;

extern int new_keyboard_data;
extern unsigned keyboard_modifiers;
extern uint8_t keyboard_scan_code[KEYBOARD_MAX_PRESSED_KEYS];

int HIDsetMachine(usb_speed_t, uint8_t,
                  usb_interface_descriptor_t const *,
                  usb_hid_main_descriptor_t const *,
                  usb_endpoint_descriptor_t const *,
                  unsigned);
int HIDisDeviceReady(void);

#ifdef __cplusplus
}
#endif

#endif
