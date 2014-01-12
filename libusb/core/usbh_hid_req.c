#include <usbh_core.h>
#include <usbh_error.h>
#include <usbh_hid_req.h>

int HIDgetReportDescriptor(int synch, uint8_t *desc, uint16_t length) {
  usb_setup_packet_t setup;
  int                result;
  uint32_t           received;

  setup.bmRequestType = DEVICE_TO_HOST | STANDARD_REQUEST |
                        INTERFACE_RECIPIENT;
  setup.bRequest = GET_DESCRIPTOR;
  setup.wValue = HID_REPORT_DESCRIPTOR << 8;
  setup.wIndex = 0;
  setup.wLength = length;

  result = USBHcontrolRequest(synch, &setup, desc, &received);

  if (result == USBHLIB_SUCCESS) {
    if (received != length)
      result = USBHLIB_ERROR_IO;
  }
  return result;
}

int HIDsetIdle(int synch, uint8_t iface, uint8_t report_id,
               uint8_t interval) {
  usb_setup_packet_t setup;

  setup.bmRequestType = HOST_TO_DEVICE | INTERFACE_RECIPIENT |
                        CLASS_REQUEST;
  setup.bRequest = SET_IDLE;
  setup.wValue = (interval << 8) | report_id;
  setup.wIndex = iface;
  setup.wLength = 0;

  return USBHcontrolRequest(synch, &setup, 0, 0);
}

int HIDsetBootProtocol(int synch, uint8_t iface,
                       usb_hid_protocol_t protocol) {
  usb_setup_packet_t setup;

  setup.bmRequestType = HOST_TO_DEVICE | INTERFACE_RECIPIENT |
                        CLASS_REQUEST;
  setup.bRequest = SET_PROTOCOL;
  setup.wValue = protocol; /* 0 == boot, 1 == report */
  setup.wIndex = iface;
  setup.wLength = 0;

  return USBHcontrolRequest(synch, &setup, 0, 0);
}

int HIDsetReport(int synch, uint8_t iface, uint8_t report_id,
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

  result = USBHcontrolRequest(synch, &setup, report, &sent);

  if (result == USBHLIB_SUCCESS) {
    if (sent != length)
      result = USBHLIB_ERROR_IO;
  }
  return result;
}
