#include <usb_endianness.h>
#include <usbh_core.h>
#include <usbh_error.h>
#include <usbh_std_req.h>

/** Standard requests **/

int USBHsetDeviceAddress(int synch, uint8_t addr) {
  usb_setup_packet_t setup;

  setup.bmRequestType = HOST_TO_DEVICE | STANDARD_REQUEST |
                        DEVICE_RECIPIENT;
  setup.bRequest = SET_ADDRESS;
  setup.wValue = addr;
  setup.wIndex = 0;
  setup.wLength = 0;

  return USBHcontrolRequest(synch, &setup, 0, 0);
}

int USBHsetConfiguration(int synch, uint8_t conf) {
  usb_setup_packet_t setup;

  setup.bmRequestType = HOST_TO_DEVICE | STANDARD_REQUEST |
                        DEVICE_RECIPIENT;
  setup.bRequest = SET_CONFIGURATION;
  setup.wValue = conf;
  setup.wIndex = 0;
  setup.wLength = 0;

  return USBHcontrolRequest(synch, &setup, 0, 0);
}

int USBHclearEndpointHalt(int synch, uint8_t ep_addr) {
  int res;
  usb_setup_packet_t setup;

  setup.bmRequestType = HOST_TO_DEVICE | STANDARD_REQUEST |
                        ENDPOINT_RECIPIENT;
  setup.bRequest = CLEAR_FEATURE;
  setup.wValue = ENDPOINT_HALT;
  setup.wIndex = ep_addr;
  setup.wLength = 0;

  res = USBHcontrolRequest(synch, &setup, 0, 0);
  return res;
}

int USBHgetDeviceDescriptor(int synch, uint8_t *desc, uint16_t length) {
  usb_setup_packet_t setup;
  int                result;
  uint32_t           received;

  setup.bmRequestType = DEVICE_TO_HOST | STANDARD_REQUEST |
                        DEVICE_RECIPIENT;
  setup.bRequest = GET_DESCRIPTOR;
  setup.wValue = DEVICE_DESCRIPTOR << 8;
  setup.wIndex = 0;
  setup.wLength = length;

  result = USBHcontrolRequest(synch, &setup, desc, &received);

  if (result == USBHLIB_SUCCESS) {
    usb_device_descriptor_t *dev_desc;
    dev_desc = (usb_device_descriptor_t *)desc;
    if (received != length ||
        (received >= 2 &&
         dev_desc->bDescriptorType != DEVICE_DESCRIPTOR)) {
      result = USBHLIB_ERROR_IO;
    }
    else {
      if (length >= 4)
        dev_desc->bcdUSB = USBTOHS(dev_desc->bcdUSB);
      if (length >= 10)
        dev_desc->idVendor = USBTOHS(dev_desc->idVendor);
      if (length >= 12)
        dev_desc->idProduct = USBTOHS(dev_desc->idProduct);
      if (length >= 14)
        dev_desc->bcdDevice = USBTOHS(dev_desc->bcdDevice);
    }
  }
  return result;
}

int USBHgetConfDescriptor(int synch, uint8_t idx, uint8_t *desc,
                          uint16_t length) {
  usb_setup_packet_t setup;
  int                result;
  uint32_t           received;

  setup.bmRequestType = DEVICE_TO_HOST | STANDARD_REQUEST |
                        DEVICE_RECIPIENT;
  setup.bRequest = GET_DESCRIPTOR;
  setup.wValue = (CONFIGURATION_DESCRIPTOR << 8) | idx;
  setup.wIndex = 0;
  setup.wLength = length;

  result = USBHcontrolRequest(synch, &setup, desc, &received);

  if (result == USBHLIB_SUCCESS) {
    usb_configuration_descriptor_t *cfg_desc;
    cfg_desc = (usb_configuration_descriptor_t *)desc;
    if (received != length ||
        (received >= 2 &&
         cfg_desc->bDescriptorType != CONFIGURATION_DESCRIPTOR)) {
      result = USBHLIB_ERROR_IO;
    }
    else if (length >= 4) {
      cfg_desc->wTotalLength = USBTOHS(cfg_desc->wTotalLength);
    }
  }
  return result;
}

int USBHgetStringDescriptorASCII(int synch, uint8_t idx, char *desc,
                                 unsigned *length) {
  static usb_max_string_descriptor_t string;
  usb_setup_packet_t setup;
  int                result;
  uint32_t           received;

  setup.bmRequestType = DEVICE_TO_HOST | STANDARD_REQUEST |
                        DEVICE_RECIPIENT;
  setup.bRequest = GET_DESCRIPTOR;
  setup.wValue = (STRING_DESCRIPTOR << 8) | idx;
  setup.wIndex = LANG_US_ENGLISH;
  setup.wLength = sizeof(usb_max_string_descriptor_t);

  result = USBHcontrolRequest(synch, &setup, (uint8_t *)&string,
                              &received);

  if (result == USBHLIB_SUCCESS) {
    if (received >= 2 && received == string.bLength &&
        string.bDescriptorType == STRING_DESCRIPTOR) {
      unsigned i, n;
      uint16_t c;
      n = (string.bLength >> 1) - 1;
      if (n > *length)
        n = *length;
      for (i = 0; i < n; ++i) {
        c = USBTOHS(string.bString[i]);
        desc[i] = c < 127 ? (char)c : '?';
      }
      *length = n;
    }
    else {
      result = USBHLIB_ERROR_IO;
    }
  }
  return result;
}
