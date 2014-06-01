#include <usb/core/usbh_core.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_hid_req.h>

int usbh_hid_get_report_descriptor(int synch, uint8_t *desc, uint16_t length) {
  usb_setup_packet_t setup;
  int                result;
  uint32_t           received;

  setup.bmRequestType = DEVICE_TO_HOST | STANDARD_REQUEST |
                        INTERFACE_RECIPIENT;
  setup.bRequest = GET_DESCRIPTOR;
  setup.wValue = HID_REPORT_DESCRIPTOR << 8;
  setup.wIndex = 0;
  setup.wLength = length;

  result = usbh_control_request(synch, &setup, desc, &received);

  if (result == USBHLIB_SUCCESS) {
    if (received != length)
      result = USBHLIB_ERROR_IO;
  }
  return result;
}

int usbh_hid_set_idle(int synch, uint8_t iface, uint8_t report_id,
               uint8_t interval) {
  usb_setup_packet_t setup;

  setup.bmRequestType = HOST_TO_DEVICE | INTERFACE_RECIPIENT |
                        CLASS_REQUEST;
  setup.bRequest = SET_IDLE;
  setup.wValue = (interval << 8) | report_id;
  setup.wIndex = iface;
  setup.wLength = 0;

  return usbh_control_request(synch, &setup, 0, 0);
}

int usbh_hid_set_boot_protocol(int synch, uint8_t iface,
                       usb_hid_protocol_t protocol) {
  usb_setup_packet_t setup;

  setup.bmRequestType = HOST_TO_DEVICE | INTERFACE_RECIPIENT |
                        CLASS_REQUEST;
  setup.bRequest = SET_PROTOCOL;
  setup.wValue = protocol; /* 0 == boot, 1 == report */
  setup.wIndex = iface;
  setup.wLength = 0;

  return usbh_control_request(synch, &setup, 0, 0);
}

int usbh_hid_set_report(int synch, uint8_t iface, uint8_t report_id,
                 uint8_t *report, uint16_t length) {
  usb_setup_packet_t setup;
  int                result;
  uint32_t           sent;

  setup.bmRequestType = HOST_TO_DEVICE | INTERFACE_RECIPIENT |
                        CLASS_REQUEST;
  setup.bRequest = SET_REPORT;
  setup.wValue = 0x0200 | report_id; /* output report */
  setup.wIndex = iface;
  setup.wLength = length;

  result = usbh_control_request(synch, &setup, report, &sent);

  if (result == USBHLIB_SUCCESS) {
    if (sent != length)
      result = USBHLIB_ERROR_IO;
  }
  return result;
}
